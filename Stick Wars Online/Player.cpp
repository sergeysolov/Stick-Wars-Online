#include "Player.h"

#include "ConnectionHandler.h"

ControlledUnit::ControlledUnit(const std::shared_ptr<Unit>& unit, const int id, const std::string& name)
{
	unit_ = unit;
	star_sprite_.setTexture(texture_holder.get_texture(star));
	star_sprite_.setScale(star_scale);

	name_text_.setFont(text_font);
	name_text_.setString(name);
	name_text_.setScale(name_scale);

	if (client_handler == nullptr)
		is_mine_ = id == 0;
	else
		is_mine_ = id == client_handler->get_id();
}

std::shared_ptr<Unit> ControlledUnit::get_unit() const
{
	return unit_;
}

bool ControlledUnit::get_is_mine() const
{
	return is_mine_;
}

void ControlledUnit::release()
{
	unit_ = nullptr;
}

void ControlledUnit::draw(DrawQueue& queue)
{
	if (unit_ != nullptr)
	{
		star_sprite_.setPosition(unit_->get_sprite().getPosition());
		star_sprite_.move(star_shift);

		name_text_.setPosition(unit_->get_sprite().getPosition());
		name_text_.move(name_shift);

		queue.emplace(attributes_layer_1, &name_text_);
		if(is_mine_)
			queue.emplace(attributes_layer_1, &star_sprite_);
	}
}

void ControlledUnit::heal() const
{
	if (unit_ != nullptr)
		unit_->cause_damage(-unit_->get_max_health() * heal_factor, 0);
}



ControlledUnit& ControlledUnit::operator=(const std::shared_ptr<Unit>& new_unit)
{
	unit_ = new_unit;
	return *this;
}

texture_ID Player::get_correct_texture_id(const texture_ID texture_id, const size_t player_id)
{
	return static_cast<texture_ID>(static_cast<int>(texture_id) + player_id);
}

Player::Player(const size_t player_id, const std::string& name) : player_id_(player_id)
{
	army_ = std::make_unique<Army>(Army::defend_lines[player_id], player_id);
	spawn_queue_ = std::make_unique<SpawnUnitQueue>(*army_);
	user_interface_ = std::make_unique<UserInterface>();

	// Add first unit of the game to player control
	auto unit_to_control = std::make_shared<Swordsman>(sf::Vector2f{ 300, 650 }, get_correct_texture_id(my_swordsman, player_id));
	controlled_unit_ = std::make_unique<ControlledUnit>(unit_to_control, player_id, name);
	army_->add_unit(unit_to_control);
}

void Player::set_screen_place(const float camera_position) const
{
	army_->set_screen_place(camera_position);
}

void Player::draw(DrawQueue& draw_queue) const
{
	army_->draw(draw_queue);
	controlled_unit_->draw(draw_queue);
	if(controlled_unit_->get_is_mine())
		user_interface_->draw(draw_queue);
}

void Player::handle_input(const Input& input, const sf::Time delta_time)
{
	if (controlled_unit_->get_unit() != nullptr)
	{
		if (controlled_unit_->get_unit()->is_alive())
		{
			const sf::Vector2i direction = { static_cast<int>(input.d) - static_cast<int>(input.a),
		static_cast<int>(input.s) - static_cast<int>(input.w) };

			if (direction.x != 0 or direction.y != 0)
			{
				controlled_unit_->get_unit()->move(direction, delta_time);
				controlled_unit_->last_position = controlled_unit_->get_unit()->get_coords();
			}
			if (input.space)
				controlled_unit_->get_unit()->commit_attack();
			else if (input.k)
				controlled_unit_->get_unit()->cause_damage(1E+10, 0);
		}
		else
			controlled_unit_->release();
	}

	if (not input.mouse_left)
		return;

	for (int i = 0; const auto& unit_buy_button : user_interface_->get_unit_buy_buttons())
	{
		if (unit_buy_button->check_mouse_pressed(input.mouse_position))
		{
			std::shared_ptr<Unit> unit;
			if (i == Miner::id and money_ >= Miner::cost)
				unit = std::static_pointer_cast<Unit>(std::make_shared<Miner>(spawn_point, get_correct_texture_id(my_miner, player_id_)));
			else if (i == Swordsman::id and money_ >= Swordsman::cost)
				unit = std::static_pointer_cast<Unit>(std::make_shared<Swordsman>(spawn_point, get_correct_texture_id(my_swordsman, player_id_)));

			if (unit != nullptr and spawn_queue_->get_free_places() >= unit->get_places_requires())
			{
				spawn_queue_->put_unit(unit, unit->get_wait_time());
				money_ -= unit->get_cost();
				unit_buy_button->press();
			}
			return;
		}
		i++;
	}

	if (user_interface_->get_defend_button()->check_mouse_pressed(input.mouse_position))
	{
		army_->set_army_target(Army::defend);
		user_interface_->get_defend_button()->press();
		return;
	}
	if (user_interface_->get_in_attack_button()->check_mouse_pressed(input.mouse_position))
	{
		army_->set_army_target(Army::attack);
		user_interface_->get_in_attack_button()->press();
		return;
	}

	bool changed_controlled_unit = false;
	for (const auto& unit : army_->get_units())
		if (unit->get_sprite().getGlobalBounds().contains(static_cast<float>(input.mouse_position.x), static_cast<float>(input.mouse_position.y)))
		{
			*controlled_unit_ = unit;
			changed_controlled_unit = true;
			break;
		}
	if (not changed_controlled_unit)
		*controlled_unit_ = nullptr;
}

std::optional<sf::Vector2f> Player::get_controlled_unit_position() const
{
	const auto position = controlled_unit_->last_position;
	controlled_unit_->last_position = {};
	return position;
}

Army& Player::get_Army() const
{
	return *army_;
}

size_t Player::get_id() const
{
	return player_id_;
}

void Player::write_to_packet(sf::Packet& packet) const
{
	packet << player_id_ << money_ << spawn_queue_->get_army_count();
	army_->write_to_packet(packet);
	if (controlled_unit_->get_unit() != nullptr)
	{
		const auto unit_it = std::ranges::find(army_->get_units(), controlled_unit_->get_unit());
		const int unit_idx = unit_it - army_->get_units().begin();
		packet << unit_idx;
	}
	else
		packet << -1;
	user_interface_->write_to_packet(packet);
}

void Player::update_from_packet(sf::Packet& packet)
{
	int army_count;
	packet >> player_id_ >> money_ >> army_count;
	army_->update_from_packet(packet);
	int controlled_unit_idx;
	packet >> controlled_unit_idx;
	if (controlled_unit_idx >= 0)
	{
		*controlled_unit_ = army_->get_units()[controlled_unit_idx];
		if (abs(controlled_unit_->get_unit()->get_speed().x) > 1e-5)
			controlled_unit_->last_position = controlled_unit_->get_unit()->get_coords();
	}
	else
		controlled_unit_->release();
	user_interface_->update_from_packet(packet);
	user_interface_->update(money_, army_count, {}, sf::Time(sf::milliseconds(0)));
}

void Player::update(const sf::Time delta_time, Army& enemy_army, const std::shared_ptr<Statue>& enemy_statue, std::vector<std::shared_ptr<GoldMine>>& gold_mines)
{
	// handle money increment
	timer_money_increment_ += delta_time.asMilliseconds();
	if (timer_money_increment_ >= time_money_increment)
	{
		timer_money_increment_ -= time_money_increment;
		money_ += count_money_increment;

		controlled_unit_->heal();
	}

	//army processing
	spawn_queue_->process(delta_time);
	money_ += army_->process(std::vector{&enemy_army}, enemy_statue, controlled_unit_->get_unit(), gold_mines, delta_time);
	user_interface_->update(money_, spawn_queue_->get_army_count(), spawn_queue_->get_front_unit_id(), delta_time);
}
