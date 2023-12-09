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

	miner_buy_button_->draw(main_window_);
	swordsman_buy_button_->draw(main_window_);
	in_attack_button_->draw(main_window_);
	defend_button_->draw(main_window_);

	main_window_.draw(stick_man_);
	main_window_.draw(army_count_text_);
	main_window_.draw(camera_position_text_);

	for (const auto mine : gold_mines_)
		mine->draw(main_window_);

	for (const auto army : armies_)
		for (const auto unit : army)
			if (not unit->is_alive())
				unit->draw(main_window_);

	for (const auto enemy : enemy_army_)
		if (not enemy->is_alive())
			enemy->draw(main_window_);

	for (const auto army : armies_)
		for (const auto unit : army)
			if (unit->is_alive())
				unit->draw(main_window_);

	for (const auto enemy : enemy_army_)
		if(enemy->is_alive())
			enemy->draw(main_window_);
	
}

void Game::process_events()
{
	auto Key_Manage = [&](sf::Event event, bool isPressed)
	{
		if (event.key.code == sf::Keyboard::D)
			is_pressed_d_ = isPressed;
		if (event.key.code == sf::Keyboard::A)
			is_pressed_a_ = isPressed;
		if (event.key.code == sf::Keyboard::W)
			is_pressed_w_ = isPressed;
		if (event.key.code == sf::Keyboard::S)
			is_pressed_s_ = isPressed;
		if (event.key.code == sf::Keyboard::K)
			is_pressed_k_ = isPressed;
		if (event.key.code == sf::Keyboard::Left)
			is_pressed_left_arrow_ = isPressed;
		if (event.key.code == sf::Keyboard::Right)
			is_pressed_right_arrow_ = isPressed;
		if (event.key.code == sf::Keyboard::Space)
			is_pressed_space_ = isPressed;
		if (event.key.code == sf::Keyboard::LShift)
			is_pressed_shift_ = isPressed;
		if (event.key.code == sf::Keyboard::Escape)
			main_window_.close();
	};

	sf::Event event;

	is_mouse_left_button_clicked_ = false;
	while (main_window_.pollEvent(event))
	{
		switch (event.type)
		{
		case sf::Event::KeyPressed:
			Key_Manage(event, true);
			break;
		case sf::Event::KeyReleased:
			Key_Manage(event, false);
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
			controlled_unit_->move(direction, sf::Time(sf::milliseconds(deltatime.asMilliseconds() * 1.5f)));
			const int shift = (controlled_unit_->get_sprite().getPosition().x + 15 - main_window_.getSize().x / 2) / 15;
			move_camera(shift);
		}
		else if (is_pressed_space_)
			controlled_unit_->commit_attack();
		else if (is_pressed_k_)
			controlled_unit_->couse_damage(1E+10);

	}

	//process mouse clicks
	if (is_mouse_left_button_clicked_)
	{
		if (miner_buy_button_->get_sprite().getGlobalBounds().contains(mouse_position_.x, mouse_position_.y)
			and money_ >= miner_buy_button_->get_unit_cost())
		{
			if (army_count_ + Miner::places_requres <= total_defend_places)
			{
				add_money(-1 * miner_buy_button_->get_unit_cost());
				units_queue_.emplace(Miner::MakeMiner(spawnpoint, texture_holder_));
				army_count_ += Miner::places_requres;
				miner_buy_button_->press();
			}
		}
		else if (swordsman_buy_button_->get_sprite().getGlobalBounds().contains(mouse_position_.x, mouse_position_.y)
			and money_ >= swordsman_buy_button_->get_unit_cost())
		{
			if (army_count_ + Swordsman::places_requres <= total_defend_places)
			{
				add_money(-1 * swordsman_buy_button_->get_unit_cost());
				units_queue_.emplace(Swordsman::MakeSwordsman(spawnpoint, texture_holder_));
				army_count_ += Swordsman::places_requres;
				swordsman_buy_button_->press();
			}
		}
		else if (defend_button_->get_sprite().getGlobalBounds().contains(mouse_position_.x, mouse_position_.y))
			set_army_target(armies_[0], Target::defend);
		else if (in_attack_button_->get_sprite().getGlobalBounds().contains(mouse_position_.x, mouse_position_.y))
			set_army_target(armies_[0], Target::attack);
		else
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

void Game::process_internal_actions(sf::Time deltatime)
{
	timer_money_increment_ += deltatime.asMilliseconds();
	if (timer_money_increment_ >= time_money_increment_)
	{
		timer_money_increment_ -= time_money_increment_;
		add_money(count_money_increment_);
	}
	const std::string temp_str = std::to_string(army_count_) + "/" + std::to_string(total_defend_places);
	army_count_text_.setString(temp_str);
	
	if (not units_queue_.empty())
	{
		if (dynamic_cast<Miner*> (units_queue_.front().get()) != nullptr)
			miner_buy_button_->process_button(deltatime.asMilliseconds());
		else if (dynamic_cast<Swordsman*> (units_queue_.front().get()) != nullptr)
			swordsman_buy_button_->process_button(deltatime.asMilliseconds());

		cumulative_spawn_time_ += deltatime.asMilliseconds();
		if (cumulative_spawn_time_ >= units_queue_.front()->get_spawn_time())
		{
			armies_[0].push_back(units_queue_.front());
			units_queue_.front()->set_target(current_target_);
			cumulative_spawn_time_ -= units_queue_.front()->get_spawn_time();
			units_queue_.pop();
		}
	}
	else
	{
		miner_buy_button_->process_button(deltatime.asMilliseconds());
		swordsman_buy_button_->process_button(deltatime.asMilliseconds());
	}

	for (auto it = gold_mines_.begin(); it != gold_mines_.end();)
	{
		if (it->get()->empty())
			it = gold_mines_.erase(it);
		else
			++it;
	}
	for (const auto& goldmine : gold_mines_)
		goldmine->set_screen_place(camera_position_);
	

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
			add_enemy_unit(std::shared_ptr<Unit>(Swordsman::EnemySwordsman(enemy_spawnpoint, texture_holder_)));
		}
		if (random(0.00005f))
		{
			set_army_target(enemy_army_, Target::attack);
		}
		else if (random(0.00005f))
		{
			set_army_target(enemy_army_, Target::defend);
		}
		if (random(0.00005f))
		{
			for (int i = 0; i < 3 and enemy_army_count_ < max_enemy_army_size_; ++i)
			{
				add_enemy_unit(std::shared_ptr<Unit>(Swordsman::EnemySwordsman(enemy_spawnpoint, texture_holder_)));
			}
		}
	}

	for (const auto& enemy : enemy_army_)
	{
		process_unit(enemy, armies_[0], enemy_defend_places_, deltatime, false);
	}
}

std::pair<bool, float> Game::check_can_mine(const Miner* miner, const GoldMine* goldmine)
{
	const auto [dx, dy] = calculate_distances_to_mine(miner, goldmine);
	//const float dx = dist_vector.x;
	//const float dy = dist_vector.y;

	//float scale_factor = (a * miner->get_coords().y + b);
	//scale_factor *= scale_factor;
	const float dist = sqrt(dx * dx + dy * dy);

	bool can_mine = false;
	if (((dx > 0 and miner->get_direction() == 1) or (dx < 0 and miner->get_direction() == -1)) and abs(dy) <= miner->get_attack_distance() and abs(dx) <= miner->get_attack_distance())
		can_mine = true;
	return {can_mine, dist};
}

sf::Vector2f Game::calculate_distances_to_mine(const Miner* miner, const GoldMine* goldmine)
{
	const float dx = goldmine->get_coords().x - miner->get_coords().x;
	const float dy = 15 * (goldmine->get_coords().y - (miner->get_coords().y) - 145);
	return { dx, dy };
}

sf::Vector2i Game::calculate_direction_to_unit(const std::shared_ptr<Unit>& unit, const std::shared_ptr<Unit>& target_unit)
{
	const float dx = target_unit->get_coords().x - unit->get_coords().x;
	const float dy = target_unit->get_coords().y - unit->get_coords().y;
	const int x_direction = dx > 0 ? 1 : -1;
	const int y_direction = dy > 0 ? 1 : -1;
	const sf::Vector2i direction = { abs(dx) > 3 ? x_direction : 0, abs(dy) > 3 ? y_direction : 0};
	return direction;
}

sf::Vector2f Game::calculate_dx_dy_between_units(const std::shared_ptr<Unit>& unit, const std::shared_ptr<Unit>& target_unit)
{
	const float dx = target_unit->get_coords().x - unit->get_coords().x;
	const float dy = target_unit->get_coords().y - unit->get_coords().y;
	return { dx, 5 * dy };
}

void Game::process_unit(const std::shared_ptr<Unit> unit, std::vector<std::shared_ptr<Unit>>& enemy_army, std::map<int, sf::Vector2f>& defend_places, sf::Time deltatime, const bool unit_from_my_army)
{
	unit->set_screen_place(camera_position_);
	if (not unit->animation_complete())
	{
		unit->add_time(deltatime.asMilliseconds());
		unit->show_animation();
	}

	if (unit->is_killed())
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

	// Process attack action
	if (unit->can_do_damage())
	{
		if (dynamic_cast<Miner*>(unit.get()) != nullptr)
		{
			auto nearest_goldmine = gold_mines_.end();
			float nearest_distance = 1E+15f;
			for (auto it = gold_mines_.begin(); it != gold_mines_.end(); ++it)
			{
				const auto [can_mine,dist] = check_can_mine(static_cast<const Miner*>(unit.get()), it->get());
				if (can_mine and dist < nearest_distance)
				{
					nearest_distance = dist;
					nearest_goldmine = it;
				}
			}
			if (nearest_goldmine != gold_mines_.end() and unit_from_my_army)
				add_money(nearest_goldmine->get()->mine(unit->get_damage()));
		}
		else if (not enemy_army.empty())
			damage_processing(unit, enemy_army);
	}

	if (unit == controlled_unit_)
		return;

	//Process logic of bots
	if (dynamic_cast<Miner*>(unit.get()) != nullptr and not gold_mines_.empty())
	{
		const auto miner = static_cast<Miner*> (unit.get());
		if (miner->attached_goldmine != nullptr)
		{
			const auto [can_mine, dist] = check_can_mine(miner, miner->attached_goldmine.get());
			if (can_mine)
			{
				if (miner->attached_goldmine->empty())
					miner->attached_goldmine = nullptr;
				else
					miner->commit_attack();
			}
			else
			{
				const sf::Vector2f distance_to_goldmine = calculate_distances_to_mine(miner, miner->attached_goldmine.get());
				if (abs(distance_to_goldmine.x) > 10)
					miner->move({ distance_to_goldmine.x > 0 ? 1 : -1, 0 }, deltatime);
				else
					miner->move({ 0, distance_to_goldmine.y > 0 ? 1 : -1 }, deltatime);
			}
		}
		else
		{
			float min_dist = 1E+15;
			auto nearest_goldmine = gold_mines_.end();
			for (auto it = gold_mines_.begin(); it != gold_mines_.end(); ++it)
			{
				const float dist = check_can_mine(miner, it->get()).second;
				if (dist < min_dist)
				{
					min_dist = dist;
					nearest_goldmine = it;
				}
			}
			miner->attached_goldmine = *nearest_goldmine;
		}
	}
	else
	{
		const int can_attack = unit_can_attack(unit, enemy_army);
		if (can_attack == 1)
			unit->commit_attack();
		else if (can_attack == -1)
		{
			unit->move({ -unit->get_direction(), 0 }, sf::Time(sf::milliseconds(1)));
			unit->commit_attack();
		}
		else
		{
			if (unit->get_target() == Target::defend)
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
					if(unit_from_my_army)
					{
						if (unit->get_direction() < 0)
							unit->move({1, 0}, sf::Time(sf::milliseconds(1)));
					}
					else
					{
						if (unit->get_direction() > 0)
							unit->move({ -1, 0 }, sf::Time(sf::milliseconds(1)));
					}
				}
			}
			else if (unit->get_target() == Target::attack)
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

void Game::damage_processing(const std::shared_ptr<Unit>& unit, std::vector<std::shared_ptr<Unit>>& enemy_army) const
{
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
		nearest_enemy->get()->couse_damage(unit->get_damage() * damage_multiplier);
}

int Game::unit_can_attack(const std::shared_ptr<Unit>& unit, std::vector<std::shared_ptr<Unit>>& enemy_army) const
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

}

void Game::init()
{
	texture_holder_.append(ID::forest_background, "Images/backgrounds/forest.png");
	texture_holder_.append(ID::large_forest_background, "Images/backgrounds/large_forest.png");

	texture_holder_.append(ID::miner, "Images/units/miner.png");
	texture_holder_.append(ID::miner_enemy, "Images/units/miner_enemy.png");
	texture_holder_.append(ID::swordsman, "Images/units/swordsman.png");
	texture_holder_.append(ID::swordsman_enemy, "Images/units/swordsman_enemy.png");

	texture_holder_.append(ID::gold, "Images/attributes/gold.png");
	texture_holder_.append(ID::miner_buy_button, "Images/attributes/miner_buy_button.png");
	texture_holder_.append(ID::stick_man, "Images/attributes/stick_man.png");
	texture_holder_.append(ID::swordsman_buy_button, "Images/attributes/swardsman_buy_button.png");
	texture_holder_.append(ID::in_attack_button, "Images/attributes/in_attack_button.png");
	texture_holder_.append(ID::defend_button, "Images/attributes/defend_button.png");

	texture_holder_.append(ID::goldmine, "Images/objects/goldmine.png");

	background_sprite_.setTexture(texture_holder_.getTexture(ID::large_forest_background));	
	background_sprite_.setTextureRect({ start_camera_position, 0 ,2100, 1050 });
	gold_sprite_.setTexture(texture_holder_.getTexture(ID::gold));
	gold_sprite_.setPosition({ 20, 20 });
	gold_sprite_.setScale({ 0.1, 0.1 });

	text_font_.loadFromFile("Images/fonts/textfont.ttf");
	money_count_text_.setFont(text_font_);
	money_count_text_.setPosition(20, 70);
	add_money(1000);

	stick_man_.setTexture(texture_holder_.getTexture(ID::stick_man));
	stick_man_.setScale({ 0.25, 0.25 });
	stick_man_.setPosition({ 35, 120 });
	army_count_text_.setFont(text_font_);
	army_count_text_.setPosition({ 25, 170 });
	army_count_text_.setString("0/" + std::to_string(total_defend_places));
	camera_position_text_.setFont(text_font_);
	camera_position_text_.setPosition(1800, 10);
	camera_position_text_.setString("x: " + std::to_string(camera_position_));

	
	miner_buy_button_.reset(new UnitBuyButton(miner_cost, miner_wait_time, { 130, 20 }, { 0.15f, 0.15f }, ID::miner_buy_button, texture_holder_, text_font_));
	swordsman_buy_button_.reset(new UnitBuyButton(swardsman_cost, swardsman_wait_time, { 230, 20 }, { 0.15f, 0.15f }, ID::swordsman_buy_button, texture_holder_, text_font_));
	defend_button_.reset(new Button({ 900.0f, 20.0f }, { 0.15f, 0.15f }, ID::defend_button, texture_holder_));
	in_attack_button_.reset(new Button({ 1000.0f, 20.0f }, { 0.15f, 0.15f }, ID::in_attack_button, texture_holder_));

	armies_.emplace_back();


	// Add first unit of the game to player controll
	armies_[0].emplace_back(Miner::MakeMiner({ 300, 650 }, texture_holder_));
	army_count_ += Miner::places_requres;

	controlled_unit_ = armies_[0][0];

	for (int i = 0; i < total_defend_places; i++)
	{
		defend_places_.insert({ i, { defendline_x - (i / 5) * row_width, y_map_max - 30 - (i % max_solders_in_row) * (y_map_max - y_map_min - 50) / max_solders_in_row } });
		enemy_defend_places_.insert({ total_defend_places - i, {enemy_defendline_x - (i / 5) * row_width, y_map_max - 30 - (i % max_solders_in_row) * (y_map_max - y_map_min - 50) / max_solders_in_row }});
	}
	for (const auto goldmine_position : goldmine_positions)
		add_gold_mine(goldmine_position, texture_holder_);

}

int Game::run()
{
	main_window_.setFramerateLimit(120);
	while (main_window_.isOpen())
	{
		sf::Time deltatime = this->clock_.restart();
		process_events();
		handle_inputs(deltatime);
		process_internal_actions(deltatime);
		
		this->draw();
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
	
	background_sprite_.setTextureRect({ camera_position_, 0, 2100, 1050 });

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

Button::Button(sf::Vector2f position, sf::Vector2f scale, ID id, TextureHolder& holder)
{
	sprite_.setTexture(holder.getTexture(id));
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

UnitBuyButton::UnitBuyButton(int unit_cost, int wait_time, sf::Vector2f position, sf::Vector2f scale, ID id, TextureHolder& holder, sf::Font& font)
	: Button(position, scale, id, holder), unit_cost_(unit_cost), wait_time_(wait_time)
{
	timebar_.setPosition({ position.x + 3, position.y + 100 });
	timebar_.setFillColor(sf::Color::Cyan);
	timebar_.setSize({ 0, 0 });

	count_text_.setFont(font);
	count_text_.setFillColor(sf::Color::Black);
	count_text_.setPosition({ position.x + 10, position.y + 50 });

	gold_icon_.setTexture(holder.getTexture(ID::gold));
	gold_icon_.setScale({ 0.04, 0.04 });
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

void UnitBuyButton::process_button(const int elapsed_time)
{
	if (remaining_time_ >= elapsed_time)
		remaining_time_ -= elapsed_time;
	else
		remaining_time_ = 0;

	std::string temp_str = "x" + std::to_string((int)ceil(static_cast<float>(remaining_time_) / wait_time_));
	count_text_.setString(temp_str.c_str());

	int remainig_time_for_current_unit = remaining_time_ % wait_time_;
	timebar_.setSize({ bar_size_.x * remainig_time_for_current_unit / wait_time_, bar_size_.y });
}

bool random(float probability)
{
	if (probability <= 0)
		return false;
	else if (probability >= 1)
		return true;

	const int p = 1 + rand() % 100000;
	return p <= static_cast<int>(probability * 100000);
}
