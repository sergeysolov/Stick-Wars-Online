#include "Game.h"
#include <iostream>
#include <functional>
#include <cmath>

std::shared_ptr<Unit> ControlledUnit::get_unit() const
{
	return unit_;
}

void ControlledUnit::release()
{
	unit_ = nullptr;
}

void ControlledUnit::draw(sf::RenderWindow& window)
{
	if (unit_ != nullptr)
	{
		star_sprite_.setPosition(unit_->get_sprite().getPosition());
		star_sprite_.move(star_shift);
		window.draw(star_sprite_);
	}
}

void ControlledUnit::heal() const
{
	if (unit_ != nullptr)
		unit_->cause_damage(-unit_->get_max_health() * health_increment);
}


ControlledUnit::ControlledUnit(TextureHolder& holder, const std::shared_ptr<Unit>& unit)
{
	unit_ = unit;
	star_sprite_.setTexture(holder.get_texture(star));
	star_sprite_.setScale(star_scale);
}

ControlledUnit& ControlledUnit::operator=(const std::shared_ptr<Unit>& new_unit)
{
	unit_ = new_unit;
	return *this;
}

void Game::draw()
{
	main_window_.clear();

	main_window_.draw(background_sprite_);

	my_statue_->draw(main_window_);
	enemy_statue_->draw(main_window_);

	for (const auto& gold_mine : gold_mines_)
		gold_mine->draw(main_window_);

	for (const auto& ally_army : armies_)
		ally_army.draw(main_window_);

	controlled_unit_->draw(main_window_);

	enemy_army_.draw(main_window_);

	user_interface_->draw(main_window_);
}

void Game::process_events()
{
	auto key_manage = [&](const sf::Event event, const bool is_pressed)
	{
		if (event.key.code == sf::Keyboard::D)
			pressed_keys_.d = is_pressed;
		if (event.key.code == sf::Keyboard::A)
			pressed_keys_.a = is_pressed;
		if (event.key.code == sf::Keyboard::W)
			pressed_keys_.w = is_pressed;
		if (event.key.code == sf::Keyboard::S)
			pressed_keys_.s = is_pressed;
		if (event.key.code == sf::Keyboard::K)
			pressed_keys_.k = is_pressed;
		if (event.key.code == sf::Keyboard::Left)
			pressed_keys_.left_arrow = is_pressed;
		if (event.key.code == sf::Keyboard::Right)
			pressed_keys_.right_arrow = is_pressed;
		if (event.key.code == sf::Keyboard::Space)
			pressed_keys_.space = is_pressed;
		if (event.key.code == sf::Keyboard::LShift)
			pressed_keys_.shift = is_pressed;
		if (event.key.code == sf::Keyboard::Escape)
			main_window_.close();
	};

	sf::Event event{};

	pressed_keys_.mouse_left = false;
	while (main_window_.pollEvent(event))
	{
		switch (event.type)
		{
		case sf::Event::KeyPressed:
			key_manage(event, true);
			break;
		case sf::Event::KeyReleased:
			key_manage(event, false);
			break;
		case sf::Event::MouseButtonReleased:
			if (event.mouseButton.button == sf::Mouse::Left)
			{
				pressed_keys_.mouse_left = true;
				mouse_position_ = sf::Mouse::getPosition(main_window_);
			}
			break;
		case sf::Event::Closed:
			main_window_.close();
		default:
			break;
		}
	}
}

void Game::handle_inputs(const sf::Time delta_time)
{
	// process behaviour of controlled unit
	if (controlled_unit_->get_unit() != nullptr)
	{
		if (controlled_unit_->get_unit()->is_alive())
		{
			const sf::Vector2i direction = { static_cast<int>(pressed_keys_.d) - static_cast<int>(pressed_keys_.a),
		static_cast<int>(pressed_keys_.s) - static_cast<int>(pressed_keys_.w) };

			if (direction.x != 0 or direction.y != 0)
			{
				const sf::Time effective_move_time = sf::milliseconds(ControlledUnit::speed_boost_factor * delta_time.asMilliseconds());
				controlled_unit_->get_unit()->move(direction, effective_move_time);
				const float shift = (controlled_unit_->get_unit()->get_coords().x + 15 - main_window_.getSize().x / 2 - camera_position_) / 15;
				move_camera(shift);
			}
			else if (pressed_keys_.space)
				controlled_unit_->get_unit()->commit_attack();
			else if (pressed_keys_.k)
				controlled_unit_->get_unit()->cause_damage(1E+10);
		}
		else
			controlled_unit_->release();
	}

	//process mouse clicks
	if (pressed_keys_.mouse_left)
	{
		if(not user_interface_->process_left_mouse_button_press(mouse_position_))
		{
			bool changed_controlled_unit = false;
			for (const auto& unit : armies_[0].get_units())
				if (unit->get_sprite().getGlobalBounds().contains(static_cast<float>(mouse_position_.x),static_cast<float>(mouse_position_.y)))
				{
					*controlled_unit_ = unit;
					changed_controlled_unit = true;
					break;
				}
			if (not changed_controlled_unit)
				*controlled_unit_ = nullptr;
		}
	}
	if (pressed_keys_.left_arrow or pressed_keys_.right_arrow)
	{
		const int direction = -static_cast<int> (pressed_keys_.left_arrow) + static_cast<int> (pressed_keys_.right_arrow);
		const int shift = direction * delta_time.asMilliseconds() * 3;
		move_camera(static_cast<float>(shift));
	}
}

void Game::set_objects_screen_place() const
{
	for (const auto& army : armies_)
		army.set_screen_place(camera_position_);

	enemy_army_.set_screen_place(camera_position_);

	for (const auto& goldmine : gold_mines_)
		goldmine->set_screen_place(camera_position_);

	my_statue_->set_screen_place(camera_position_);
	enemy_statue_->set_screen_place(camera_position_);
}

void Game::process_internal_actions(const sf::Time delta_time)
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
		process_enemy_spawn_queue(*enemy_spawn_queue_, texture_holder_);

	enemy_army_.process(armies_[0], my_statue_, nullptr, gold_mines_, delta_time);

	set_objects_screen_place();
}


Game::Game(const uint16_t width, const uint16_t height, const char* title)
	: main_window_(sf::VideoMode(width, height), title), enemy_army_(Army::enemy_defend_line, false)
{
	texture_holder_.load_textures();

	background_sprite_.setTexture(texture_holder_.get_texture(large_forest_background));
	background_sprite_.setTextureRect({ static_cast<int>(start_camera_position), 0 ,static_cast<int>(map_frame_width), 1050 });

	my_statue_ = std::make_shared<Statue>(Statue::my_statue_position, texture_holder_, my_statue, Statue::my_max_health);
	enemy_statue_ = std::make_shared<Statue>(Statue::enemy_statue_position, texture_holder_, enemy_statue, Statue::enemy_max_health);
	//My army
	armies_.emplace_back(Army::defend_line_1, true);

	//Creating spawn queues
	my_spawn_queue_ = std::make_unique<SpawnUnitQueue>(armies_[0]);
	enemy_spawn_queue_ = std::make_unique<SpawnUnitQueue>(enemy_army_);

	// Add first unit of the game to player control

	auto unit_to_control = std::make_shared<Swordsman>(sf::Vector2f{ 300, 650 }, texture_holder_, my_swordsman);
	controlled_unit_ = std::make_unique<ControlledUnit>(texture_holder_, unit_to_control);
	armies_[0].add_unit(unit_to_control);

	// Add goldmines
	for (const auto goldmine_position : GoldMine::goldmine_positions)
		gold_mines_.emplace_back(new GoldMine(goldmine_position, texture_holder_));

	user_interface_ = std::make_unique<UserInterface>(texture_holder_, money_, camera_position_, armies_[0], *my_spawn_queue_);
}

int Game::run()
{
	main_window_.setFramerateLimit(120);
	while (main_window_.isOpen())
	{
		const sf::Time delta_time = this->clock_.restart();
		process_events();
		handle_inputs(delta_time);
		process_internal_actions(delta_time);
		draw();
		main_window_.display();
	}
	return 0;
}

void Game::move_camera(const float step)
{
	camera_position_ = std::clamp(camera_position_ + step, min_camera_position, max_camera_position);
	background_sprite_.setTextureRect({ static_cast<int>(camera_position_), 0, static_cast<int>(map_frame_width), 1050 });
}