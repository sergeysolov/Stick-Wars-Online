#include "Game.h"
#include <iostream>
#include <functional>
#include <cmath>

void Game::draw()
{
	main_window_.clear();
	main_window_.draw(background_sprite_);
	main_window_.draw(gold_sprite_);
	main_window_.draw(money_count_text_);

	my_statue_->draw(main_window_);
	enemy_statue_->draw(main_window_);

	for (const auto& unit_button : unit_buy_buttons_)
		unit_button->draw(main_window_);

	in_attack_button_->draw(main_window_);
	defend_button_->draw(main_window_);

	main_window_.draw(stick_man_);
	main_window_.draw(army_count_text_);
	main_window_.draw(camera_position_text_);

	for (const auto& mine : gold_mines_)
		mine->draw(main_window_);

	for (const auto& army : armies_)
		for (const auto& unit : army)
			if (not unit->is_alive())
				unit->draw(main_window_);

	for (const auto& enemy : enemy_army_)
		if (not enemy->is_alive())
			enemy->draw(main_window_);

	for (const auto& army : armies_)
		for (const auto& unit : army)
			if (unit->is_alive())
				unit->draw(main_window_);

	for (const auto& enemy : enemy_army_)
		if(enemy->is_alive())
			enemy->draw(main_window_);
	
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

void Game::handle_inputs(sf::Time deltatime)
{
	// process behaviour of controlled unit
	if (controlled_unit_ != nullptr and controlled_unit_->is_alive())
	{
		const sf::Vector2i direction = { static_cast<int>(is_pressed_d_) - static_cast<int>(is_pressed_a_),
		static_cast<int>(is_pressed_s_) - static_cast<int>(is_pressed_w_) };

		if (direction.x != 0 or direction.y != 0)
		{
			controlled_unit_->move(direction, sf::Time(sf::milliseconds(1.5f * deltatime.asMilliseconds())));
			const int shift = (controlled_unit_->get_sprite().getPosition().x + 15 - main_window_.getSize().x / 2.f) / 15;
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
		const bool unit_buy_button_pressed = process_unit_buttons();
		if (defend_button_->get_sprite().getGlobalBounds().contains(mouse_position_.x, mouse_position_.y))
			set_army_target(armies_[0], defend);
		else if (in_attack_button_->get_sprite().getGlobalBounds().contains(mouse_position_.x, mouse_position_.y))
			set_army_target(armies_[0], attack);
		else if(not unit_buy_button_pressed)
		{
			bool changed_controlled_unit = false;
			for (const auto& unit : armies_[0])
				if (unit->is_alive() and unit->get_sprite().getGlobalBounds().contains(mouse_position_.x, mouse_position_.y))
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
		const int shift = direction * deltatime.asMilliseconds() * 3;
		move_camera(shift);
	}
}

void Game::set_objects_screen_place() const
{
	for (const auto& army : armies_)
		for (const auto& unit : army)
			unit->set_screen_place(camera_position_);

	for (const auto& enemy : enemy_army_)
		enemy->set_screen_place(camera_position_);

	for (const auto& goldmine : gold_mines_)
		goldmine->set_screen_place(camera_position_);

	my_statue_->set_screen_place(camera_position_);
	enemy_statue_->set_screen_place(camera_position_);
}

void Game::process_internal_actions(const sf::Time deltatime)
{
	// handle money increment
	timer_money_increment_ += deltatime.asMilliseconds();
	if (timer_money_increment_ >= time_money_increment_)
	{
		timer_money_increment_ -= time_money_increment_;
		add_money(count_money_increment_);
	}

	// update army count text
	const std::string temp_str = std::to_string(army_count_) + "/" + std::to_string(total_defend_places);
	army_count_text_.setString(temp_str);

	//process unit's queue and units spawn
	if (not units_queue_.empty())
	{
		//process button
		for (const auto& unit_button : unit_buy_buttons_)
			if(unit_button->get_unit_id() == units_queue_.front()->get_id())
				unit_button->process_button(deltatime.asMilliseconds());

		cumulative_spawn_time_ += deltatime.asMilliseconds();
		if (cumulative_spawn_time_ >= units_queue_.front()->get_spawn_time())
		{
			armies_[0].push_back(units_queue_.front());
			units_queue_.front()->set_target(current_target_);
			cumulative_spawn_time_ -= units_queue_.front()->get_spawn_time();
			units_queue_.pop();
		}
	}

	set_objects_screen_place();

	//My army processing
	for (const auto& unit : armies_[0])
	{
		process_unit(unit, enemy_army_, defend_places_, deltatime, true);
	}

	//Enemy army processing
	if (enemy_behaviour == 0)
	{
		cumulative_enemy_spawn_time_ += deltatime.asMilliseconds();
		if (cumulative_enemy_spawn_time_ >= invoke_enemy_time and enemy_army_count_ < max_enemy_army_size_)
		{
			cumulative_enemy_spawn_time_ -= invoke_enemy_time;
			add_enemy_unit(std::make_shared<Swordsman>(enemy_spawnpoint, texture_holder_, enemy_swordsman));
		}
		if (random(0.0001f))
		{
			set_army_target(enemy_army_, attack);
		}
		else if (random(0.00005f))
		{
			set_army_target(enemy_army_, defend);
		}
		if (random(0.0001f))
		{
			for (int i = 0; i < 3 and enemy_army_count_ < max_enemy_army_size_; ++i)
			{
				add_enemy_unit(std::make_shared<Swordsman>(enemy_spawnpoint, texture_holder_, enemy_swordsman));
			}
		}
	}

	for (const auto& enemy : enemy_army_)
	{
		process_unit(enemy, armies_[0], enemy_defend_places_, deltatime, false);
	}
}


sf::Vector2i Game::calculate_direction_to_unit(const std::shared_ptr<Unit>& unit, const std::shared_ptr<Unit>& target_unit)
{
	const float dx = target_unit->get_coords().x - unit->get_coords().x;
	const float dy = target_unit->get_coords().y - unit->get_coords().y;
	const int x_direction = dx > 0 ? 1 : -1;
	const int y_direction = dy > 0 ? 1 : -1;
	const sf::Vector2i direction = { abs(dx) > 3 ? x_direction : 0, abs(dy) > 3 and abs(dx) < 200 ? y_direction : 0};
	return direction;
}

sf::Vector2f Game::calculate_dx_dy_between_units(const std::shared_ptr<Unit>& unit, const std::shared_ptr<Unit>& target_unit)
{
	const float dx = target_unit->get_coords().x - unit->get_coords().x;
	const float dy = target_unit->get_coords().y - unit->get_coords().y;
	return { dx, 5 * dy };
}

void Game::process_unit(const std::shared_ptr<Unit>& unit, std::vector<std::shared_ptr<Unit>>& enemy_army, std::map<int, sf::Vector2f>& defend_places, sf::Time deltatime, const bool unit_from_my_army)
{
	unit->show_animation(deltatime.asMilliseconds());

	// process unit if it was killed
	if (unit->was_killed())
	{
		auto stand_place = unit->extract_stand_place();
		if (stand_place.first >= 0)
			defend_places.insert(stand_place);

		if (unit_from_my_army)
			army_count_ -= unit->get_places_requres();
		else
			enemy_army_count_ -= unit->get_places_requres();
	}

	//Give stand place with less number to unit if place become free
	if (not defend_places.empty() and unit->get_stand_place().first > defend_places.begin()->first)
	{
		auto stand_place = unit->extract_stand_place();
		unit->set_stand_place(defend_places);
		defend_places.insert(stand_place);
	}

	if (not unit->is_alive())
		return;


	if (const auto miner = dynamic_cast<Miner*>(unit.get()); miner != nullptr)
		process_miner(miner, deltatime);
	else
	{
		damage_processing(unit, enemy_army);

		if (unit == controlled_unit_)
			return;

		const int can_attack = is_enemy_nearby(unit, enemy_army);
		if (can_attack == 1)
			unit->commit_attack();
		else if (can_attack == -1)
		{
			unit->move({ -unit->get_direction(), 0 }, sf::Time(sf::milliseconds(1)));
			unit->commit_attack();
		}
		else
		{
			if (unit->get_target() == defend)
			{
				if (unit->get_stand_place().second.x > 1E+10)
					unit->set_stand_place(defend_places);

				const float distance_x = unit->get_stand_place().second.x - unit->get_coords().x;
				const float distance_y = unit->get_stand_place().second.y - unit->get_coords().y;
				if (abs(distance_x) + abs(distance_y) > 5)
				{
					const sf::Vector2i direction = { distance_x > 0 ? 1 : -1, distance_y > 0 ? 1 : -1 };
					unit->move(direction, deltatime);
				}
				else
				{
					if (unit_from_my_army)
					{
						if (unit->get_direction() < 0)
							unit->move({ 1, 0 }, sf::Time(sf::milliseconds(1)));
					}
					else
					{
						if (unit->get_direction() > 0)
							unit->move({ -1, 0 }, sf::Time(sf::milliseconds(1)));
					}
				}
			}
			else if (unit->get_target() == attack)
			{
				if (unit->target_unit == nullptr or not unit->target_unit->is_alive())
					unit->target_unit = find_nearest_enemy_unit(unit, enemy_army);
				if (unit->target_unit != nullptr)
					unit->move(calculate_direction_to_unit(unit, unit->target_unit), deltatime);
			}
		}
	}
}

std::shared_ptr<Unit> Game::find_nearest_enemy_unit(const std::shared_ptr<Unit>& unit, std::vector<std::shared_ptr<Unit>> army) const
{
	auto nearest_enemy = army.end();
	float nearest_distance = 1E+15f;
	for (auto it = army.begin(); it != army.end(); ++it)
	{
		if (not it->get()->is_alive())
			continue;

		const float dx = it->get()->get_coords().x - unit->get_coords().x;
		const float dy = it->get()->get_coords().y - unit->get_coords().y;

		const float distance = abs(dx) + 2 * abs(dy);

		if (distance < nearest_distance)
		{
			nearest_distance = distance;
			nearest_enemy = it;
		}
	}
	if (nearest_enemy != army.end())
		return *nearest_enemy;
	return nullptr;
}

void Game::process_miner(Miner* miner, const sf::Time delta_time)
{
	if(gold_mines_.empty())
		return;

	auto check_can_mine = [&](const GoldMine* goldmine) -> std::pair<bool, sf::Vector2f>
		{
			const float dx = goldmine->get_coords().x - miner->get_coords().x;
			const float dy = 10 * (goldmine->get_coords().y - miner->get_coords().y - 120); // * 15,   -145

			bool can_mine = dx * static_cast<float>(miner->get_direction()) > 0 and abs((dy - 145)) <= miner->get_attack_distance() and abs(dx) <= miner->get_attack_distance();
			return { can_mine, {dx, dy} };
		};

	auto nearest_goldmine = gold_mines_.end();
	auto find_nearest_goldmine = [&]
		{
			if (nearest_goldmine == gold_mines_.end())
			{
				float nearest_distance = 1E+15f;
				for (auto it = gold_mines_.begin(); it != gold_mines_.end(); ++it)
				{
					const auto [_, dist_vector] = check_can_mine(it->get());
					const auto dist = abs(dist_vector.x) + abs(dist_vector.y);
					if (dist < nearest_distance)
					{
						nearest_distance = dist;
						nearest_goldmine = it;
					}
				}
			}
			return nearest_goldmine;
		};

	if (miner->can_do_damage() and check_can_mine(find_nearest_goldmine()->get()).first)
	{
		const int gold_count = find_nearest_goldmine()->get()->mine(static_cast<int>(miner->get_damage()));
		
		if (miner->get_id() != enemy_miner)
			add_money(gold_count);
	}

	if (miner != controlled_unit_.get())
	{
		if (miner->attached_goldmine != nullptr)
		{
			const auto [can_mine, distance_to_goldmine] = check_can_mine(miner->attached_goldmine.get());
			if (can_mine)
				miner->commit_attack();
			else
			{
				int x_direction = distance_to_goldmine.x > 0 ? 1 : -1;
				if (abs(distance_to_goldmine.x) > 180)
					miner->move({ x_direction, 0 }, delta_time);
				else
				{
					int y_direction = distance_to_goldmine.y > 0 ? 1 : -1;
					miner->move({ x_direction, y_direction }, delta_time);
				}
			}
			if (miner->attached_goldmine->empty())
				miner->attached_goldmine.reset();
		}
		else
			miner->attached_goldmine = *find_nearest_goldmine();
	}
	if (find_nearest_goldmine()->get()->empty())
		gold_mines_.erase(find_nearest_goldmine());
}

void Game::damage_processing(const std::shared_ptr<Unit>& unit, std::vector<std::shared_ptr<Unit>>& enemy_army) const
{
	if (not unit->can_do_damage() or enemy_army.empty())
		return;

	auto nearest_enemy = enemy_army.end();
	sf::Vector2f nearest_distance = { 1E+15, 1E+15 };
	float damage_multiplier = 0;
	for (auto it = enemy_army.begin(); it != enemy_army.end(); ++it)
	{
		if (not it->get()->is_alive())
			continue;

		const auto [dx, dy] = calculate_dx_dy_between_units(unit, *it);

		if (dx > 0 and unit->get_direction() == 1)
		{
			if (unit->get_direction() + it->get()->get_direction() == 0)
				damage_multiplier = 1;
			else if (unit->get_direction() + it->get()->get_direction() == 2)
				damage_multiplier = 2;
		}
		else if (dx < 0 and unit->get_direction() == -1)
		{
			if (unit->get_direction() + it->get()->get_direction() == 0)
				damage_multiplier = 1;
			else if (unit->get_direction() + it->get()->get_direction() == -2)
				damage_multiplier = 2;
		}

		const float distance = abs(dx) + abs(dy);

		if (distance < abs(nearest_distance.x) + abs(nearest_distance.y) and damage_multiplier != 0)
		{
			nearest_distance = {dx, dy };
			nearest_enemy = it;
		}
	}
	if (unit == controlled_unit_)
		damage_multiplier *= 2;
	if (nearest_enemy != enemy_army.end() and abs(nearest_distance.x) <= unit->get_attack_distance() and abs(nearest_distance.y) <= unit->get_attack_distance())
		nearest_enemy->get()->cause_damage(unit->get_damage() * damage_multiplier);

}

int Game::is_enemy_nearby(const std::shared_ptr<Unit>& unit, std::vector<std::shared_ptr<Unit>>& enemy_army) const
{
	for (auto it = enemy_army.begin(); it != enemy_army.end(); ++it)
	{
		if (it->get()->is_alive())
		{
			int side = 0;
			const auto [dx, dy] = calculate_dx_dy_between_units(unit, *it);

			side = (dx > 0 ? 1 : -1) * unit->get_direction();
			if (abs(dx) <= unit->get_attack_distance() and abs(dy) <= unit->get_attack_distance())
				return side;
		}	
	}
	return 0;
}

void Game::set_army_target(const std::vector<std::shared_ptr<Unit>>& army, const Target target)
{
	for (const auto& unit : army)
		unit->set_target(target);
	if (army == armies_[0])
		current_target_ = target;
	else
		current_enemy_target_ = target;
}

void Game::add_money(const int count)
{
	money_ += count;
	money_count_text_.setString(std::to_string(money_).c_str());
}


Game::Game(const uint16_t width, const uint16_t height, const char* title)
	: main_window_(sf::VideoMode(width, height), title)
{
	texture_holder_.load_textures();

	background_sprite_.setTexture(texture_holder_.get_texture(large_forest_background));
	background_sprite_.setTextureRect({ static_cast<int>(start_camera_position), 0 ,static_cast<int>(map_frame_width), 1050 });
	gold_sprite_.setTexture(texture_holder_.get_texture(ID::gold));
	gold_sprite_.setPosition({ 20, 20 });
	gold_sprite_.setScale({ 0.1, 0.1 });

	text_font_.loadFromFile("Images/fonts/textfont.ttf");
	money_count_text_.setFont(text_font_);
	money_count_text_.setPosition(20, 70);
	add_money(1000);

	stick_man_.setTexture(texture_holder_.get_texture(ID::stick_man));
	stick_man_.setScale({ 0.25, 0.25 });
	stick_man_.setPosition({ 35, 120 });
	army_count_text_.setFont(text_font_);
	army_count_text_.setPosition({ 25, 170 });
	army_count_text_.setString("0/" + std::to_string(total_defend_places));
	camera_position_text_.setFont(text_font_);
	camera_position_text_.setPosition(1800, 10);
	camera_position_text_.setString("x: " + std::to_string(camera_position_));

	unit_buy_buttons_.push_back(std::make_unique<UnitBuyButton>(my_miner, Miner::cost, Miner::wait_time, sf::Vector2f{ 130, 20 }, sf::Vector2f{ 0.15f, 0.15f }, ID::miner_buy_button, texture_holder_, text_font_));
	unit_buy_buttons_.push_back(std::make_unique<UnitBuyButton>(my_swordsman, Swordsman::cost, Swordsman::wait_time, sf::Vector2f{ 230, 20 }, sf::Vector2f{ 0.15f, 0.15f }, ID::swordsman_buy_button, texture_holder_, text_font_));

	defend_button_.reset(new Button({ 900.0f, 20.0f }, { 0.15f, 0.15f }, ID::defend_button, texture_holder_));
	in_attack_button_.reset(new Button({ 1000.0f, 20.0f }, { 0.15f, 0.15f }, ID::in_attack_button, texture_holder_));

	my_statue_.reset(new Statue({ 500, 450 }, texture_holder_, my_statue, Statue::my_max_health));
	enemy_statue_.reset(new Statue({ map_frame_width * 3 - 800, 450 }, texture_holder_, enemy_statue, Statue::enemy_max_health));

	armies_.emplace_back();

	// Add first unit of the game to player control
	armies_[0].emplace_back(new Miner({ 300, 650 }, texture_holder_, my_miner));
	army_count_ += Miner::places_requres;

	controlled_unit_ = armies_[0][0];

	for (int i = 0; i < total_defend_places; i++)
	{
		defend_places_.insert({ i, { defendline_x - (i / 5) * row_width, y_map_max - 30 - (i % max_soldiers_in_row) * (y_map_max - y_map_min - 50) / max_soldiers_in_row } });
		enemy_defend_places_.insert({ total_defend_places - i, {enemy_defendline_x - (i / 5) * row_width, y_map_max - 30 - (i % max_soldiers_in_row) * (y_map_max - y_map_min - 50) / max_soldiers_in_row } });
	}
	for (const auto goldmine_position : goldmine_positions)
		add_gold_mine(goldmine_position, texture_holder_);
}

int Game::run()
{
	main_window_.setFramerateLimit(120);
	while (main_window_.isOpen())
	{
		const sf::Time deltatime = this->clock_.restart();
		process_events();
		handle_inputs(deltatime);
		process_internal_actions(deltatime);
		draw();
		main_window_.display();
	}
	return 0;
}

int Game::move_camera(const int step)
{
	const int prev_camera_position = camera_position_;
	camera_position_ += step;
	if (camera_position_ < min_camera_position)
		camera_position_ = -min_camera_position;
	else if (camera_position_ > max_camera_position)
		camera_position_ = max_camera_position;
	const int actual_step = camera_position_ - prev_camera_position;
	
	background_sprite_.setTextureRect({ static_cast<int>(camera_position_), 0, static_cast<int>(map_frame_width), 1050 });

	camera_position_text_.setString("x: " + std::to_string(camera_position_));
	return actual_step;
}

void Game::add_enemy_unit(const std::shared_ptr<Unit>& unit)
{
	unit->set_target(current_enemy_target_);
	enemy_army_.push_back(unit);
	enemy_army_count_ += unit->get_places_requres();
}

void Game::add_gold_mine(const sf::Vector2f position, TextureHolder& holder)
{
	gold_mines_.emplace_back(new GoldMine(position, holder));
}

bool Game::process_unit_buttons()
{
	for (const auto& unit_button : unit_buy_buttons_)
	{
		if (unit_button->get_sprite().getGlobalBounds().contains(mouse_position_.x, mouse_position_.y)
			and money_ >= unit_button->get_unit_cost())
		{
			
			if(unit_button->get_unit_id() == Miner::texture_id)
			{
				units_queue_.emplace(new Miner(spawnpoint, texture_holder_, unit_button->get_unit_id()));
				army_count_ += Miner::places_requres;
				
			}
			else if(unit_button->get_unit_id() == Swordsman::texture_id)
			{
				units_queue_.emplace(new Swordsman(spawnpoint, texture_holder_, unit_button->get_unit_id()));
				army_count_ += Swordsman::places_requres;
			}
			add_money(-1 * unit_button->get_unit_cost());
			unit_button->press();
			return true;
		}
	}
	return false;
}

Button::Button(sf::Vector2f position, sf::Vector2f scale, ID id, TextureHolder& holder)
{
	sprite_.setTexture(holder.get_texture(id));
	sprite_.setPosition(position);
	sprite_.setScale(scale);
}

const sf::Sprite& Button::get_sprite() const
{
	return sprite_;
}

void Button::draw(sf::RenderWindow& window) const
{
	window.draw(sprite_);
}

void Button::press()
{
	pressed_ = true;
}

bool Button::is_pressed()
{
	const bool temp = pressed_;
	pressed_ = false;
	return temp;
}

UnitBuyButton::UnitBuyButton(ID unit_id, int unit_cost, int wait_time, sf::Vector2f position, sf::Vector2f scale, ID id, TextureHolder& holder, const sf::Font& font)
	: Button(position, scale, id, holder), wait_time_(wait_time), unit_cost_(unit_cost), unit_id_(unit_id)
{
	timebar_.setPosition({ position.x + 3, position.y + 100 });
	timebar_.setFillColor(sf::Color::Cyan);
	timebar_.setSize({ 0, 0 });

	count_text_.setFont(font);
	count_text_.setFillColor(sf::Color::Black);
	count_text_.setPosition({ position.x + 10, position.y + 50 });

	gold_icon_.setTexture(holder.get_texture(ID::gold));
	gold_icon_.setScale({ 0.04f, 0.04f });
	gold_icon_.setPosition({ position.x + 5, position.y + 10 });

	cost_text_.setFont(font);
	cost_text_.setString(std::to_string(unit_cost).c_str());
	cost_text_.setFillColor(sf::Color::Black);
	cost_text_.setPosition({ position.x + 40, position.y + 10 });
	cost_text_.setCharacterSize(20);
}

void UnitBuyButton::draw(sf::RenderWindow& window) const
{
	window.draw(sprite_);
	window.draw(timebar_);
	window.draw(gold_icon_);
	window.draw(cost_text_);
	if(remaining_time_ != 0)
		window.draw(count_text_);
}

void UnitBuyButton::press()
{
	remaining_time_ += wait_time_;
	this->process_button(1);
}

int UnitBuyButton::get_unit_cost() const
{
	return unit_cost_;
}

ID UnitBuyButton::get_unit_id() const
{
	return unit_id_;
}

void UnitBuyButton::process_button(const int elapsed_time)
{
	if (remaining_time_ >= elapsed_time)
		remaining_time_ -= elapsed_time;
	else
		remaining_time_ = 0;

	const std::string temp_str = "x" + std::to_string(static_cast<int>(ceil(static_cast<float>(remaining_time_) / wait_time_)));
	count_text_.setString(temp_str.c_str());

	const int remaining_time_for_current_unit = remaining_time_ % wait_time_;
	timebar_.setSize({ bar_size_.x * remaining_time_for_current_unit / wait_time_, bar_size_.y });
}

bool random(const float probability)
{
	if (probability <= 0)
		return false;
	if (probability >= 1)
		return true;

	static std::mt19937 generator{ std::random_device{}()};
	static std::uniform_real_distribution distribution(0.0f, 1.0f);

	return distribution(generator) < probability;
}
