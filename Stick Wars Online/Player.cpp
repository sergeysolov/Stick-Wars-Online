#include "Player.h"

ControlledUnit::ControlledUnit(const std::shared_ptr<Unit>& unit)
{
	unit_ = unit;
	star_sprite_.setTexture(texture_holder.get_texture(star));
	star_sprite_.setScale(star_scale);
}

std::shared_ptr<Unit> ControlledUnit::get_unit() const
{
	return unit_;
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

Player::Player(const size_t player_id) : player_id_(player_id), army_(Army::defend_lines[player_id], true)
{
	spawn_queue_ = std::make_unique<SpawnUnitQueue>(army_);

	// Add first unit of the game to player control
	auto unit_to_control = std::make_shared<Swordsman>(sf::Vector2f{ 300, 650 }, my_swordsman);
	controlled_unit_ = std::make_unique<ControlledUnit>(unit_to_control);
	army_.add_unit(unit_to_control);
}

void Player::set_screen_place(const float camera_position) const
{
	army_.set_screen_place(camera_position);
}

void Player::handle_mouse_input(const sf::Vector2i mouse_position) const
{
	bool changed_controlled_unit = false;
	for (const auto& unit : army_.get_units())
		if (unit->get_sprite().getGlobalBounds().contains(static_cast<float>(mouse_position.x), static_cast<float>(mouse_position.y)))
		{
			*controlled_unit_ = unit;
			changed_controlled_unit = true;
			break;
		}
	if (not changed_controlled_unit)
		*controlled_unit_ = nullptr;
}

std::optional<sf::Vector2f> Player::handle_keyboard_input(const Input& input, const sf::Time delta_time) const
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
				return { controlled_unit_->get_unit()->get_coords() };
			}
			if (input.space)
				controlled_unit_->get_unit()->commit_attack();
			else if (input.k)
				controlled_unit_->get_unit()->cause_damage(1E+10, 0);
		}
		else
			controlled_unit_->release();
	}
	return {};
}

void Player::draw(DrawQueue& draw_queue) const
{
	army_.draw(draw_queue);
	controlled_unit_->draw(draw_queue);
}

Army& Player::get_Army()
{
	return army_;
}

int& Player::get_money_count()
{
	return money_;
}

SpawnUnitQueue& Player::get_SpawnQueue() const
{
	return *spawn_queue_;
}

void Player::update(const sf::Time delta_time, const Army& enemy_army, const std::shared_ptr<Statue>& enemy_statue, std::vector<std::shared_ptr<GoldMine>>& gold_mines)
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
	money_ += army_.process(enemy_army, enemy_statue, controlled_unit_->get_unit(), gold_mines, delta_time);
}
