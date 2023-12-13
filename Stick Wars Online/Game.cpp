#include "Game.h"
#include <iostream>
#include <functional>
#include <cmath>

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

	enemy_army_.draw(main_window_);

	user_interface_->draw(main_window_);
}

void Game::process_events()
{
	auto key_manage = [&](const sf::Event event, const bool is_pressed)
	{
		if (event.key.code == sf::Keyboard::D)
			is_pressed_d_ = is_pressed;
		if (event.key.code == sf::Keyboard::A)
			is_pressed_a_ = is_pressed;
		if (event.key.code == sf::Keyboard::W)
			is_pressed_w_ = is_pressed;
		if (event.key.code == sf::Keyboard::S)
			is_pressed_s_ = is_pressed;
		if (event.key.code == sf::Keyboard::K)
			is_pressed_k_ = is_pressed;
		if (event.key.code == sf::Keyboard::Left)
			is_pressed_left_arrow_ = is_pressed;
		if (event.key.code == sf::Keyboard::Right)
			is_pressed_right_arrow_ = is_pressed;
		if (event.key.code == sf::Keyboard::Space)
			is_pressed_space_ = is_pressed;
		if (event.key.code == sf::Keyboard::LShift)
			is_pressed_shift_ = is_pressed;
		if (event.key.code == sf::Keyboard::Escape)
			main_window_.close();
	};

	sf::Event event{};

	is_mouse_left_button_clicked_ = false;
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
				is_mouse_left_button_clicked_ = true;
				mouse_position_ = sf::Mouse::getPosition();
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
	if (controlled_unit_ != nullptr and controlled_unit_->is_alive())
	{
		const sf::Vector2i direction = { static_cast<int>(is_pressed_d_) - static_cast<int>(is_pressed_a_),
		static_cast<int>(is_pressed_s_) - static_cast<int>(is_pressed_w_) };

		if (direction.x != 0 or direction.y != 0)
		{
			const sf::Time effective_move_time = sf::milliseconds(2 * delta_time.asMilliseconds());
			controlled_unit_->move(direction, effective_move_time);
			const float shift = (controlled_unit_->get_sprite().getPosition().x + 15 - static_cast<float>(main_window_.getSize().x) / 2.f) / 15;
			move_camera(shift);
		}
		else if (is_pressed_space_)
			controlled_unit_->commit_attack();
		else if (is_pressed_k_)
			controlled_unit_->cause_damage(1E+10);
	}

	//process mouse clicks
	if (is_mouse_left_button_clicked_)
	{
		if(not user_interface_->process_left_mouse_button_press(mouse_position_))
		{
			bool changed_controlled_unit = false;
			for (const auto& unit : armies_[0].get_units())
				if (unit->is_alive() and unit->get_sprite().getGlobalBounds().contains(static_cast<float>(mouse_position_.x),static_cast<float>(mouse_position_.y)))
				{
					controlled_unit_ = unit;
					changed_controlled_unit = true;
					break;
				}
			if (not changed_controlled_unit)
				controlled_unit_ = nullptr;
		}
	}
	if (is_pressed_left_arrow_ or is_pressed_right_arrow_)
	{
		const int direction = -static_cast<int> (is_pressed_left_arrow_) + static_cast<int> (is_pressed_right_arrow_);
		const int shift = direction * delta_time.asMilliseconds() * 3;
		move_camera(static_cast<float>(shift));
	}
}

void Game::set_objects_screen_place() const
{
	for (const auto& army : armies_)
		for (const auto& unit : army.get_units())
			unit->set_screen_place(camera_position_);

	for (const auto& enemy : enemy_army_.get_units())
		enemy->set_screen_place(camera_position_);

	for (const auto& goldmine : gold_mines_)
		goldmine->set_screen_place(camera_position_);

	my_statue_->set_screen_place(camera_position_);
	enemy_statue_->set_screen_place(camera_position_);
}

void Game::process_internal_actions(const sf::Time delta_time)
{
	// handle money increment
	timer_money_increment_ += delta_time.asMilliseconds();
	if (timer_money_increment_ >= time_money_increment_)
	{
		timer_money_increment_ -= time_money_increment_;
		money_ += count_money_increment_;
	}

	user_interface_->update(delta_time);

	set_objects_screen_place();

	//My army processing
	my_spawn_queue_->process(delta_time);
	money_ += armies_[0].process(enemy_army_, controlled_unit_, gold_mines_, delta_time);

	//Enemy army processing
	enemy_spawn_queue_->process(delta_time);

	if (enemy_behaviour == 0)
		process_enemy_spawn_queue(*enemy_spawn_queue_, texture_holder_);

	enemy_army_.process(armies_[0], nullptr, gold_mines_, delta_time);
}


Game::Game(const uint16_t width, const uint16_t height, const char* title)
	: main_window_(sf::VideoMode(width, height), title), enemy_army_(Army::enemy_defend_line, false)
{
	texture_holder_.load_textures();

	background_sprite_.setTexture(texture_holder_.get_texture(large_forest_background));
	background_sprite_.setTextureRect({ static_cast<int>(start_camera_position), 0 ,static_cast<int>(map_frame_width), 1050 });

	my_statue_ = std::make_unique<Statue>(Statue::my_statue_position, texture_holder_, my_statue, Statue::my_max_health);
	enemy_statue_ = std::make_unique<Statue>(Statue::enemy_statue_position, texture_holder_, enemy_statue, Statue::enemy_max_health);

	//My army
	armies_.emplace_back(Army::defend_line_1, true);

	//Creating spawn queues
	my_spawn_queue_ = std::make_unique<SpawnUnitQueue>(armies_[0]);
	enemy_spawn_queue_ = std::make_unique<SpawnUnitQueue>(enemy_army_);

	// Add first unit of the game to player control
	controlled_unit_ = std::make_shared<Swordsman>(sf::Vector2f{ 300, 650 }, texture_holder_, my_swordsman);
	armies_[0].add_unit(controlled_unit_);

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
	const float prev_camera_position = camera_position_;
	camera_position_ += step;
	if (camera_position_ < min_camera_position)
		camera_position_ = -min_camera_position;
	else if (camera_position_ > max_camera_position)
		camera_position_ = max_camera_position;
	
	background_sprite_.setTextureRect({ static_cast<int>(camera_position_), 0, static_cast<int>(map_frame_width), 1050 });
}