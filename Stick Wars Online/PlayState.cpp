#include "PlayState.h"

void PlayState::set_objects_screen_place() const
{
	for (const auto& army : armies_)
		army.set_screen_place(camera_position_);

	enemy_army_.set_screen_place(camera_position_);

	for (const auto& goldmine : gold_mines_)
		goldmine->set_screen_place(camera_position_);

	my_statue_->set_screen_place(camera_position_);
	enemy_statue_->set_screen_place(camera_position_);
}

void PlayState::move_camera(const float step)
{
	camera_position_ = std::clamp(camera_position_ + step, min_camera_position, max_camera_position);
	background_sprite_.setTextureRect({ static_cast<int>(camera_position_), 0, static_cast<int>(map_frame_width), 1050 });
}

PlayState::PlayState() : enemy_army_(Army::enemy_defend_line, false)
{

	background_sprite_.setTexture(texture_holder.get_texture(large_forest_background));
	background_sprite_.setTextureRect({ static_cast<int>(start_camera_position), 0 ,static_cast<int>(map_frame_width), 1050 });

	my_statue_ = std::make_shared<Statue>(Statue::my_statue_position, my_statue, Statue::my_max_health);
	enemy_statue_ = std::make_shared<Statue>(Statue::enemy_statue_position, enemy_statue, Statue::enemy_max_health);
	//My army
	armies_.emplace_back(Army::defend_line_1, true);

	//Creating spawn queues
	my_spawn_queue_ = std::make_unique<SpawnUnitQueue>(armies_[0]);
	enemy_spawn_queue_ = std::make_unique<SpawnUnitQueue>(enemy_army_);

	// Add first unit of the game to player control

	auto unit_to_control = std::make_shared<Swordsman>(sf::Vector2f{ 300, 650 }, my_swordsman);
	controlled_unit_ = std::make_unique<ControlledUnit>(unit_to_control);
	armies_[0].add_unit(unit_to_control);

	// Add goldmines
	for (const auto goldmine_position : GoldMine::goldmine_positions)
		gold_mines_.emplace_back(new GoldMine(goldmine_position));

	user_interface_ = std::make_unique<UserInterface>(money_, camera_position_, armies_[0], *my_spawn_queue_);
}

void PlayState::update(sf::Time delta_time)
{
	// handle money increment
	timer_money_increment_ += delta_time.asMilliseconds();
	if (timer_money_increment_ >= time_money_increment)
	{
		timer_money_increment_ -= time_money_increment;
		money_ += count_money_increment_;

		controlled_unit_->heal();
	}

	user_interface_->update(delta_time);

	//My army processing
	my_spawn_queue_->process(delta_time);
	money_ += armies_[0].process(enemy_army_, enemy_statue_, controlled_unit_->get_unit(), gold_mines_, delta_time);

	//Enemy army processing
	enemy_spawn_queue_->process(delta_time);

	if (enemy_behaviour == 0)
		process_enemy_spawn_queue(*enemy_spawn_queue_);

	enemy_army_.process(armies_[0], my_statue_, nullptr, gold_mines_, delta_time);

	set_objects_screen_place();
}

void PlayState::handle_input(Input& input, const sf::Time delta_time)
{
	// process behaviour of controlled unit
	if (controlled_unit_->get_unit() != nullptr)
	{
		if (controlled_unit_->get_unit()->is_alive())
		{
			const sf::Vector2i direction = { static_cast<int>(input.d) - static_cast<int>(input.a),
		static_cast<int>(input.s) - static_cast<int>(input.w) };

			if (direction.x != 0 or direction.y != 0)
			{
				//const sf::Time effective_move_time = sf::milliseconds(ControlledUnit::speed_boost_factor * delta_time.asMilliseconds());
				controlled_unit_->get_unit()->move(direction, delta_time);
				const float window_width = static_cast<float>(sf::VideoMode::getDesktopMode().width);
				const float shift = (controlled_unit_->get_unit()->get_coords().x + 15 - window_width / 2 - camera_position_) / 15;
				move_camera(shift);
			}
			else if (input.space)
				controlled_unit_->get_unit()->commit_attack();
			else if (input.k)
				controlled_unit_->get_unit()->cause_damage(1E+10, 0);
		}
		else
			controlled_unit_->release();
	}

	//process mouse clicks
	if (input.mouse_left)
	{
		if (not user_interface_->process_left_mouse_button_press(input.mouse_position))
		{
			bool changed_controlled_unit = false;
			for (const auto& unit : armies_[0].get_units())
				if (unit->get_sprite().getGlobalBounds().contains(static_cast<float>(input.mouse_position.x), static_cast<float>(input.mouse_position.y)))
				{
					*controlled_unit_ = unit;
					changed_controlled_unit = true;
					break;
				}
			if (not changed_controlled_unit)
				*controlled_unit_ = nullptr;
		}
	}
	if (input.left_arrow or input.right_arrow)
	{
		const int direction = -static_cast<int> (input.left_arrow) + static_cast<int> (input.right_arrow);
		const int shift = direction * delta_time.asMilliseconds() * 3;
		move_camera(static_cast<float>(shift));
	}
}

void PlayState::draw(DrawQueue& draw_queue)
{
	draw_queue.emplace(background, &background_sprite_);

	my_statue_->draw(draw_queue);
	enemy_statue_->draw(draw_queue);

	for (const auto& gold_mine : gold_mines_)
		gold_mine->draw(draw_queue);

	enemy_army_.draw(draw_queue);

	for (const auto& ally_army : armies_)
		ally_army.draw(draw_queue);

	controlled_unit_->draw(draw_queue);

	user_interface_->draw(draw_queue);
}

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