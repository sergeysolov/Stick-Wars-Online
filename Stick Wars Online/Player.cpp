#include "Player.h"

#include "ConnectionHandler.h"

ControlledUnit::ControlledUnit(const std::shared_ptr<Unit>& unit, const int id, const std::string& name)
	: aim_({0, 0}, Aim::archer_scale, Aim::archer_static_shift, Aim::archer_shift)
{
	unit_ = unit;
	star_sprite_.setTexture(texture_holder.get_texture(star));
	star_sprite_.setScale(star_scale);
	star_sprite_.setOrigin({star_sprite_.getLocalBounds().width / 2,star_sprite_.getLocalBounds().height / 2});

	name_text_.setFont(text_font);
	name_text_.setString(name);
	name_text_.setScale(name_scale);
	name_text_.setOrigin({ name_text_.getLocalBounds().width / 2, name_text_.getLocalBounds().height / 2 });

	if (client_handler == nullptr)
		is_me_ = id == 0;
	else
		is_me_ = id == client_handler->get_id();
}

std::shared_ptr<Unit> ControlledUnit::get_unit() const
{
	return unit_;
}

bool ControlledUnit::get_is_me() const
{
	return is_me_;
}

int ControlledUnit::get_stun_time_left() const
{
	if (unit_ != nullptr)
		return static_cast<int>(static_cast<float>(unit_->get_stun_time_left()) / speed_boost_factor);
	return 0;
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
		star_sprite_.move(star_offset);

		name_text_.setPosition(unit_->get_sprite().getPosition());
		name_text_.move(name_offset);

		queue.emplace(attributes_layer_1, &name_text_);
		if (is_me_) {
			queue.emplace(attributes_layer_1, &star_sprite_);
			if (dynamic_cast<Archer*>(unit_.get()) != nullptr) {
				aim_.set_position(unit_->get_sprite().getPosition());
				aim_.draw(queue);
			}
		}
	}
}

void ControlledUnit::heal() const
{
	if (unit_ != nullptr)
		unit_->cause_damage(-unit_->get_max_health() * heal_factor, 0, 0);
}

void ControlledUnit::set_y_scale()
{
	if(unit_ != nullptr)
	{
		const sf::Vector2f star_scale_factors = { star_scale.x * abs(unit_->get_sprite().getScale().x), star_scale.y * unit_->get_sprite().getScale().y };
		star_sprite_.setScale(star_scale_factors);
		const sf::Vector2f name_text_scale_factors = { name_scale.x * abs(unit_->get_sprite().getScale().x), name_scale.y * unit_->get_sprite().getScale().y };
		name_text_.setScale(name_text_scale_factors);
		const sf::Vector2f aim_scale_factor = { Aim::archer_scale.x * abs(unit_->get_sprite().getScale().x), Aim::archer_scale.y * unit_->get_sprite().getScale().y };
		aim_.set_scale(aim_scale_factor);
		aim_.set_direction(unit_->get_direction());
	}
}

void ControlledUnit::move_aim(const int direction, const sf::Time delta_time)
{
	if (unit_ != nullptr) {
		if (const auto archer = dynamic_cast<Archer*>(unit_.get()); archer != nullptr) {
			const float delta_angle = -direction * delta_time.asSeconds() * Archer::bow_rotation_speed;
			aim_.move({ 0, 0 }, delta_angle);
			archer->set_bow_angle(aim_.get_angle());
		}
	}
}


ControlledUnit& ControlledUnit::operator=(const std::shared_ptr<Unit>& new_unit)
{
	unit_ = new_unit;
	return *this;
}

void Player::handle_change_controlled_unit(const sf::Vector2i mouse_position) const
{
	bool changed_controlled_unit = false;
	for (const auto& unit : army_->get_units())
		if (unit->get_unit_rect().contains(static_cast<float>(mouse_position.x), static_cast<float>(mouse_position.y)))
		{
			*controlled_unit_ = unit;
			changed_controlled_unit = true;
			break;
		}
	if (not changed_controlled_unit)
		*controlled_unit_ = nullptr;
}

texture_ID Player::get_correct_texture_id(const texture_ID texture_id, const int player_id)
{
	return static_cast<texture_ID>(static_cast<int>(texture_id) + player_id);
}

Player::Player(const int player_id, const std::string& name) : player_id_(player_id)
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
	if(controlled_unit_->get_is_me())
		user_interface_->draw(draw_queue);
}

void Player::handle_input(const Input& input, const int mouse_offset, const sf::Time delta_time)
{
	if (controlled_unit_->get_unit() != nullptr)
	{
		if (controlled_unit_->get_unit()->is_alive())
		{
			const sf::Vector2i direction = { static_cast<int>(input.d) - static_cast<int>(input.a),
											 static_cast<int>(input.s) - static_cast<int>(input.w) };
			const int aim_direction = static_cast<int>(input.up_arrow) - static_cast<int>(input.down_arrow);
			if (input.e)
				controlled_unit_->get_unit()->stand_defend();
			if (input.space)
				controlled_unit_->get_unit()->commit_attack();
			else if (input.f)
				controlled_unit_->get_unit()->commit_second_attack();
			else if (direction.x != 0 or direction.y != 0)
			{
				controlled_unit_->get_unit()->move(direction, delta_time);
				controlled_unit_->last_position = controlled_unit_->get_unit()->get_coords();
			}
			else if (input.k)
				controlled_unit_->get_unit()->cause_damage(1E+10, 0, 0);
			if (aim_direction != 0) {
				controlled_unit_->move_aim(aim_direction, delta_time);
			}
		}
		else
			controlled_unit_->release();
	}

	if (input.mouse_left)
	{
		for (int i = 0; const auto& unit_buy_button : user_interface_->get_unit_buy_buttons())
		{
			if (unit_buy_button->check_mouse_pressed(input.mouse_position))
			{
				const std::shared_ptr<Unit> unit(UnitFactory::create_unit(i, player_id_));
				if (money_ >= unit->get_cost() and spawn_queue_->get_free_places() >= unit->get_places_requires())
				{
					spawn_queue_->put_unit(unit, unit->get_wait_time());
					money_ -= unit->get_cost();
					unit_buy_button->press_left();
				}
				return;
			}
			i++;
		}

		if (user_interface_->get_defend_button()->check_mouse_pressed(input.mouse_position))
		{
			army_->set_army_target(Army::defend);
			user_interface_->get_defend_button()->press_left();
			return;
		}
		if (user_interface_->get_in_attack_button()->check_mouse_pressed(input.mouse_position))
		{
			army_->set_army_target(Army::attack);
			user_interface_->get_in_attack_button()->press_left();
			return;
		}
		if(user_interface_->get_escape_button()->check_mouse_pressed(input.mouse_position))
		{
			army_->set_army_target(Army::escape);
			user_interface_->get_escape_button()->press_left();
			return;
		}

		handle_change_controlled_unit({ input.mouse_position.x + mouse_offset, input.mouse_position.y });
	}
	else if(input.mouse_right)
	{
		for (int i = 0; const auto& unit_buy_button : user_interface_->get_unit_buy_buttons())
		{
			if (unit_buy_button->check_mouse_pressed(input.mouse_position))
			{
				if(spawn_queue_->remove_unit(i))
				{
					money_ += unit_buy_button->get_unit_cost();
					unit_buy_button->press_right();
				}
				return;
			}
			i++;
		}
	}
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
	packet << player_id_ << money_ << total_damage_ << total_kills_ << controlled_unit_->get_stun_time_left() << spawn_queue_->get_army_count();
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
	int controlled_unit_stun_time_left;
	packet >> player_id_ >> money_ >> total_damage_ >> total_kills_ >> controlled_unit_stun_time_left >> army_count;

	private_effect_manager.set_active(controlled_unit_->get_is_me());
	private_sound_manager.set_active(controlled_unit_->get_is_me());

	controlled_unit_->set_y_scale();

	army_->update_from_packet(packet);
	int controlled_unit_idx;
	packet >> controlled_unit_idx;
	if (controlled_unit_idx >= 0 and controlled_unit_idx < army_->get_units().size())
	{
		*controlled_unit_ = army_->get_units()[controlled_unit_idx];
		if (abs(controlled_unit_->get_unit()->get_speed().x) > 1e-5)
			controlled_unit_->last_position = controlled_unit_->get_unit()->get_coords();
	}
	else
		controlled_unit_->release();

	user_interface_->update_from_packet(packet, army_->get_army_target());

	user_interface_->update({ money_, total_damage_, total_kills_ }, controlled_unit_stun_time_left, army_count, {}, sf::Time(sf::milliseconds(0)));
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

	private_effect_manager.set_active(controlled_unit_->get_is_me());
	private_sound_manager.set_active(controlled_unit_->get_is_me());

	controlled_unit_->set_y_scale();

	user_interface_->update({ money_, total_damage_, total_kills_ }, controlled_unit_->get_stun_time_left(), spawn_queue_->get_army_count(), spawn_queue_->get_front_unit_id(), delta_time);

	//army processing
	spawn_queue_->process(delta_time);
	const auto [gold_count, damage, kills] = army_->process(std::vector{&enemy_army}, enemy_statue, controlled_unit_->get_unit(), gold_mines, delta_time);
	money_ += gold_count;
	total_damage_ += damage;
	total_kills_ += kills;
}

void Player::update_client_locally(const sf::Time delta_time) const
{
	army_->process_client_locally(delta_time, controlled_unit_->get_unit());
}
