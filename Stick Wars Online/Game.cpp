#include "Game.h"
#include <iostream>
#include <functional>
#include <cmath>

void Game::_draw()
{
	_main_window.clear();
	_main_window.draw(_background_sprite);
	_main_window.draw(_gold_sprite);
	_main_window.draw(_money_count_text);

	_miner_buy_button->draw(_main_window);
	_swordsman_buy_button->draw(_main_window);
	_in_attack_button->draw(_main_window);
	_defend_button->draw(_main_window);

	_main_window.draw(_stick_man);
	_main_window.draw(_army_count_text);
	_main_window.draw(_camera_position_text);

	for (const auto mine : _gold_mines)
		mine->draw(_main_window);

	for (const auto army : _armies)
		for (const auto unit : army)
			if (not unit->is_alive())
				unit->draw(_main_window);

	for (const auto enemy : _enemy_army)
		if (not enemy->is_alive())
			enemy->draw(_main_window);

	for (const auto army : _armies)
		for (const auto unit : army)
			if (unit->is_alive())
				unit->draw(_main_window);

	for (const auto enemy : _enemy_army)
		if(enemy->is_alive())
			enemy->draw(_main_window);
	
}

void Game::_process_events()
{
	std::function<void(sf::Event, bool)> Key_Manage = [&](sf::Event event, bool isPressed)
	{
		if (event.key.code == sf::Keyboard::D)
			_isPressed_D = isPressed;
		if (event.key.code == sf::Keyboard::A)
			_isPressed_A = isPressed;
		if (event.key.code == sf::Keyboard::W)
			_isPressed_W = isPressed;
		if (event.key.code == sf::Keyboard::S)
			_isPressed_S = isPressed;
		if (event.key.code == sf::Keyboard::K)
			_isPressed_K = isPressed;
		if (event.key.code == sf::Keyboard::Left)
			_is_Pressed_Left_Arrow = isPressed;
		if (event.key.code == sf::Keyboard::Right)
			_is_Pressed_Right_Arrow = isPressed;
		if (event.key.code == sf::Keyboard::Space)
			_isPressed_Space = isPressed;
		if (event.key.code == sf::Keyboard::LShift)
			_isPressed_Shift = isPressed;
		if (event.key.code == sf::Keyboard::Escape)
			_main_window.close();
	};

	sf::Event event;

	_isMouse_Left_Button_Clicked = false;
	while (_main_window.pollEvent(event))
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
				_isMouse_Left_Button_Clicked = true;
				_mouse_position = sf::Mouse::getPosition();
			}
			break;
		case sf::Event::Closed:
			_main_window.close();
		default:
			break;
		}
	}
}

void Game::_handle_inputs(sf::Time deltatime)
{
	if (_controlled_unit != nullptr and _controlled_unit->is_alive())
	{
		sf::Vector2i direction = { static_cast<int>(_isPressed_D) - static_cast<int>(_isPressed_A),
		static_cast<int>(_isPressed_S) - static_cast<int>(_isPressed_W) };

		if (direction.x != 0 or direction.y != 0)
		{
			_controlled_unit->move(direction, sf::Time(sf::milliseconds(deltatime.asMilliseconds() * 1.5f)));
			int shift = (_controlled_unit->get_sprite().getPosition().x + 15 - _main_window.getSize().x / 2) / 15;
			_move_camera(shift);
		}
		else if (_isPressed_Space)
			_controlled_unit->commit_attack();
		else if (_isPressed_K)
			_controlled_unit->couse_damage(1E+10);

	}

	if (_isMouse_Left_Button_Clicked)
	{
		if (_miner_buy_button->get_sprite().getGlobalBounds().contains(_mouse_position.x, _mouse_position.y)
			and _money >= _miner_buy_button->get_unit_cost())
		{
			if (_army_count + Miner::places_requres <= total_defend_places)
			{
				_add_money(-1 * _miner_buy_button->get_unit_cost());
				_units_queue.emplace(Miner::MyMiner(spawnpoint, _texture_holder));
				_army_count += Miner::places_requres;
				_miner_buy_button->press();
			}
		}
		else if (_swordsman_buy_button->get_sprite().getGlobalBounds().contains(_mouse_position.x, _mouse_position.y)
			and _money >= _swordsman_buy_button->get_unit_cost())
		{
			if (_army_count + Swordsman::places_requres <= total_defend_places)
			{
				_add_money(-1 * _swordsman_buy_button->get_unit_cost());
				_units_queue.emplace(Swordsman::MySwordsman(spawnpoint, _texture_holder));
				_army_count += Swordsman::places_requres;
				_swordsman_buy_button->press();
			}
		}
		else if (_defend_button->get_sprite().getGlobalBounds().contains(_mouse_position.x, _mouse_position.y))
			_set_army_target(_armies[0], Target::defend);
		else if (_in_attack_button->get_sprite().getGlobalBounds().contains(_mouse_position.x, _mouse_position.y))
			_set_army_target(_armies[0], Target::attack);
		else
		{
			bool changed_controlled_unit = false;
			for (const auto unit : _armies[0])
				if (unit->is_alive() and unit->get_sprite().getGlobalBounds().contains(_mouse_position.x, _mouse_position.y))
				{
					_controlled_unit = unit;
					changed_controlled_unit = true;
					break;
				}
			if (not changed_controlled_unit)
				_controlled_unit = nullptr;
		}

	}
	if (_is_Pressed_Left_Arrow or _is_Pressed_Right_Arrow)
	{
		int direction = -static_cast<int> (_is_Pressed_Left_Arrow) + static_cast<int> (_is_Pressed_Right_Arrow);
		int shift = direction * deltatime.asMilliseconds() * 3;
		_move_camera(shift);
	}
}

void Game::_process_internal_actions(sf::Time deltatime)
{
	_timer_money_increment += deltatime.asMilliseconds();
	if (_timer_money_increment >= _time_money_increment)
	{
		_timer_money_increment -= _time_money_increment;
		_add_money(_count_money_increment);
	}
	std::string temp_str = std::to_string(_army_count) + "/" + std::to_string(total_defend_places);
	_army_count_text.setString(temp_str);
	
	if (not _units_queue.empty())
	{
		if (dynamic_cast<Miner*> (_units_queue.front().get()) != nullptr)
			_miner_buy_button->process_button(deltatime.asMilliseconds());
		else if (dynamic_cast<Swordsman*> (_units_queue.front().get()) != nullptr)
			_swordsman_buy_button->process_button(deltatime.asMilliseconds());

		_cumulative_spawn_time += deltatime.asMilliseconds();
		if (_cumulative_spawn_time >= _units_queue.front()->get_spawn_time())
		{
			_add_unit(_units_queue.front());
			_cumulative_spawn_time -= _units_queue.front()->get_spawn_time();
			_units_queue.pop();
		}
	}
	else
	{
		_miner_buy_button->process_button(deltatime.asMilliseconds());
		_swordsman_buy_button->process_button(deltatime.asMilliseconds());
	}

	for (auto it = _gold_mines.begin(); it != _gold_mines.end();)
	{
		if (it->get()->empty())
			it = _gold_mines.erase(it);
		else
			++it;
	}
	for (const auto goldmine : _gold_mines)
		goldmine->set_screen_place(_camera_position);
	

	//My army processing
	for (const auto unit : _armies[0])
	{
		_process_unit(unit, _enemy_army, _defend_places, deltatime, true);
	}


	//Enemy army processing
	if (enemy_programm == 0)
	{
		_cumulative_enemy_spawn_time += deltatime.asMilliseconds();
		if (_cumulative_enemy_spawn_time >= invoke_enemy_time and _enemy_army_count < _max_enemy_army_size)
		{
			_cumulative_enemy_spawn_time -= invoke_enemy_time;
			_add_enemy_unit(std::shared_ptr<Unit>(Swordsman::EnemySwordsman(enemy_spawnpoint, _texture_holder)));
		}
		if (random(0.00005))
		{
			_set_army_target(_enemy_army, Target::attack);
		}
		else if (random(0.00005))
		{
			_set_army_target(_enemy_army, Target::defend);
		}
		if (random(0.00005))
		{
			for (int i = 0; i < 3 and _enemy_army_count < _max_enemy_army_size; ++i)
			{
				_add_enemy_unit(std::shared_ptr<Unit>(Swordsman::EnemySwordsman(enemy_spawnpoint, _texture_holder)));
			}
		}

	}

	for (const auto enemy : _enemy_army)
	{
		_process_unit(enemy, _armies[0], _enemy_defend_places, deltatime, false);
	}
}

std::pair<bool, float> Game::_check_can_mine(const Miner* miner, const GoldMine* goldmine)
{
	auto dist_vector = _calculate_distances_to_mine(miner, goldmine);
	float dx = dist_vector.x;
	float dy = dist_vector.y;

	//float scale_factor = (a * miner->get_coords().y + b);
	//scale_factor *= scale_factor;
	float distance = sqrt(dx * dx + 24 * dy * dy);

	bool can_mine = false;
	if (((dx > 0 and miner->get_direction() == 1) or (dx < 0 and miner->get_direction() == -1) or distance < 70) and distance <= miner->get_attack_distance())
		can_mine = true;
	return {can_mine, distance};
}

sf::Vector2f Game::_calculate_distances_to_mine(const Miner* miner, const GoldMine* goldmine)
{
	float dx = goldmine->get_coords().x - (miner->get_coords().x + miner->get_direction() * 50);
	float dy = (goldmine->get_coords().y + goldmine->get_sprite().getTextureRect().getSize().y + 30) - (miner->get_coords().y + miner->get_sprite().getTextureRect().getSize().y);
	return { dx, dy };
}

sf::Vector2i Game::_calculate_direction_to_unit(const Unit* unit, const Unit* target_unit)
{
	float dx = target_unit->get_coords().x - unit->get_coords().x;
	float dy = target_unit->get_coords().y - unit->get_coords().y;
	sf::Vector2i direction = { dx > 0 ? 1 : -1, dy > 0 ? 1 : -1 };
	return direction;
}

void Game::_process_unit(std::shared_ptr<Unit> unit, std::vector<std::shared_ptr<Unit>>& enemy_army, std::map<int, sf::Vector2f>& defend_places, sf::Time deltatime, bool unit_from_my_army)
{
	unit->set_screen_place(_camera_position);
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
			_army_count -= unit->get_places_requres();
		else
			_enemy_army_count -= unit->get_places_requres();
	}
	if (not defend_places.empty() and unit->get_stand_place().first > defend_places.begin()->first)
	{
		auto stand_place = unit->extract_stand_place();
		unit->set_stand_place(defend_places);
		defend_places.insert(stand_place);
	}

	if (not unit->is_alive())
		return;

	if (unit->can_do_damage())
	{
		if (dynamic_cast<Miner*>(unit.get()) != nullptr)
		{
			auto nearest_goldmine = _gold_mines.end();
			float nearest_distance = 1E+15;
			for (auto it = _gold_mines.begin(); it != _gold_mines.end(); ++it)
			{
				auto res = _check_can_mine(static_cast<Miner*> (unit.get()), it->get());
				if (res.first and res.second < nearest_distance)
				{
					nearest_distance = res.second;
					nearest_goldmine = it;
				}
			}
			if (nearest_goldmine != _gold_mines.end() and unit_from_my_army)
				_add_money(nearest_goldmine->get()->mine(unit->get_damage()));
		}
		else if (not enemy_army.empty())
			_damage_processing(unit, enemy_army);
	}

	if (unit == _controlled_unit)
		return;

	if (dynamic_cast<Miner*>(unit.get()) != nullptr and not _gold_mines.empty())
	{
		Miner* miner = static_cast<Miner*> (unit.get());
		if (miner->attached_goldmine != nullptr)
		{
			auto res = _check_can_mine(miner, miner->attached_goldmine.get());
			if (res.first)
			{
				if (miner->attached_goldmine->empty())
					miner->attached_goldmine = nullptr;
				else
					miner->commit_attack();
			}
			else
			{
				sf::Vector2f distance_to_goldmine = _calculate_distances_to_mine(miner, miner->attached_goldmine.get());
				if (abs(distance_to_goldmine.x) > 10)
					miner->move({ distance_to_goldmine.x > 0 ? 1 : -1, 0 }, deltatime);
				else
					miner->move({ 0, distance_to_goldmine.y > 0 ? 1 : -1 }, deltatime);
			}
		}
		else
		{
			float min_dist = 1E+15;
			auto nearest_goldmine = _gold_mines.end();
			for (auto it = _gold_mines.begin(); it != _gold_mines.end(); ++it)
			{
				float dist = _check_can_mine(miner, it->get()).second;
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
		int _can_attack = _unit_can_attack(unit, enemy_army);
		if (_can_attack == 1)
			unit->commit_attack();
		else if (_can_attack == -1)
		{
			unit->move({ -1, 0 }, sf::Time(sf::milliseconds(1)));
			unit->commit_attack();
		}
		else
		{
			if (unit->get_target() == Target::defend)
			{
				if (unit->get_stand_place().second.x > 1E+10)
					unit->set_stand_place(defend_places);

				float distance_x = unit->get_stand_place().second.x - unit->get_coords().x;
				float distance_y = unit->get_stand_place().second.y - unit->get_coords().y;
				if (abs(distance_x) + abs(distance_y) > 5)
				{
					sf::Vector2i direction = { distance_x > 0 ? 1 : -1, distance_y > 0 ? 1 : -1 };
					unit->move(direction, deltatime);
				}
			}
			else if (unit->get_target() == Target::attack)
			{
				if (unit->target_unit.get() == nullptr or not unit->target_unit->is_alive())
					unit->target_unit = _find_nearest_enemy_unit(unit, enemy_army);
				if (unit->target_unit.get() != nullptr)
				{
					auto direction = _calculate_direction_to_unit(unit.get(), unit->target_unit.get());
					if (direction.x != 0)
						unit->move({ direction.x, 0 }, deltatime);
					else
						unit->move(direction, deltatime);
				}	
			}
		}
	}
}

std::shared_ptr<Unit> Game::_find_nearest_enemy_unit(std::shared_ptr<Unit> unit, std::vector<std::shared_ptr<Unit>> army)
{
	auto nearest_enemy = army.end();
	float nearest_distance = 1E+15;
	for (auto it = army.begin(); it != army.end(); ++it)
	{
		if (not it->get()->is_alive())
			continue;

		float dx = it->get()->get_coords().x - unit->get_coords().x;
		float dy = it->get()->get_coords().y - unit->get_coords().y;

		float distance = abs(dx) + 2 * abs(dy);

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

void Game::_damage_processing(std::shared_ptr<Unit> unit, std::vector<std::shared_ptr<Unit>>& enemy_army)
{
	auto nearest_enemy = enemy_army.end();
	float nearest_distance = 1E+15;
	float damage_multiplier = 0;
	for (auto it = enemy_army.begin(); it != enemy_army.end(); ++it)
	{
		if (not it->get()->is_alive())
			continue;

		float dx = it->get()->get_coords().x - unit->get_coords().x;
		float dy = it->get()->get_coords().y - unit->get_coords().y;

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

		float distance = abs(dx) + 2 * abs(dy);

		if (distance < nearest_distance and damage_multiplier != 0)
		{
			nearest_distance = distance;
			nearest_enemy = it;
		}
	}
	if (unit == _controlled_unit)
		damage_multiplier *= 2;
	if (nearest_enemy != enemy_army.end() and nearest_distance <= unit->get_attack_distance())
		nearest_enemy->get()->couse_damage(unit->get_damage() * damage_multiplier);
}

int Game::_unit_can_attack(std::shared_ptr<Unit> unit, std::vector<std::shared_ptr<Unit>>& enemy_army) const
{
	for (auto it = enemy_army.begin(); it != enemy_army.end(); ++it)
	{
		int side = 0;
		if (not it->get()->is_alive())
			continue;

		float dx = it->get()->get_coords().x - unit->get_coords().x;
		float dy = it->get()->get_coords().y - unit->get_coords().y;

		side = (dx > 0 ? 1 : -1) * unit->get_direction();
		if (abs(dx) + 2 * abs(dy) <= unit->get_attack_distance())
			return side;
	}
	return 0;
}

void Game::_set_army_target(std::vector<std::shared_ptr<Unit>>& army, Target target)
{
	for (auto unit : army)
		unit->set_target(target);
	if (army == _armies[0])
		_current_target = target;
	else
		_current_enemy_target = target;
}

void Game::_add_money(int count)
{
	_money += count;
	_money_count_text.setString(std::to_string(_money).c_str());
}


Game::Game(uint16_t width, uint16_t height, const char* title)
	: _main_window(sf::VideoMode(width, height), title)
{

}

void Game::init()
{
	_texture_holder.append(ID::forest_background, "Images/backgrounds/forest.png");
	_texture_holder.append(ID::large_forest_background, "Images/backgrounds/large_forest.png");

	_texture_holder.append(ID::miner, "Images/units/miner.png");
	_texture_holder.append(ID::miner_enemy, "Images/units/miner_enemy.png");
	_texture_holder.append(ID::swordsman, "Images/units/swordsman.png");
	_texture_holder.append(ID::swordsman_enemy, "Images/units/swordsman_enemy.png");

	_texture_holder.append(ID::gold, "Images/attributes/gold.png");
	_texture_holder.append(ID::miner_buy_button, "Images/attributes/miner_buy_button.png");
	_texture_holder.append(ID::stick_man, "Images/attributes/stick_man.png");
	_texture_holder.append(ID::swordsman_buy_button, "Images/attributes/swardsman_buy_button.png");
	_texture_holder.append(ID::in_attack_button, "Images/attributes/in_attack_button.png");
	_texture_holder.append(ID::defend_button, "Images/attributes/defend_button.png");

	_texture_holder.append(ID::goldmine, "Images/objects/goldmine.png");

	_background_sprite.setTexture(_texture_holder.getTexture(ID::large_forest_background));	
	_background_sprite.setTextureRect({ start_camera_position, 0 ,2100, 1050 });
	_gold_sprite.setTexture(_texture_holder.getTexture(ID::gold));
	_gold_sprite.setPosition({ 20, 20 });
	_gold_sprite.setScale({ 0.1, 0.1 });

	_textfont.loadFromFile("Images/fonts/textfont.ttf");
	_money_count_text.setFont(_textfont);
	_money_count_text.setPosition(20, 70);
	_add_money(1000);

	_stick_man.setTexture(_texture_holder.getTexture(ID::stick_man));
	_stick_man.setScale({ 0.25, 0.25 });
	_stick_man.setPosition({ 35, 120 });
	_army_count_text.setFont(_textfont);
	_army_count_text.setPosition({ 25, 170 });
	_army_count_text.setString("0/" + std::to_string(total_defend_places));
	_camera_position_text.setFont(_textfont);
	_camera_position_text.setPosition(1800, 10);
	_camera_position_text.setString("x: " + std::to_string(_camera_position));

	
	_miner_buy_button.reset(new UnitBuyButton(MINER_COST, MINER_WAIT_TIME, { 130, 20 }, { 0.15f, 0.15f }, ID::miner_buy_button, _texture_holder, _textfont));
	_swordsman_buy_button.reset(new UnitBuyButton(SWARDSMAN_COST, SWARDSMAN_WAIT_TIME, { 230, 20 }, { 0.15f, 0.15f }, ID::swordsman_buy_button, _texture_holder, _textfont));
	_defend_button.reset(new Button({ 900.0f, 20.0f }, { 0.15f, 0.15f }, ID::defend_button, _texture_holder));
	_in_attack_button.reset(new Button({ 1000.0f, 20.0f }, { 0.15f, 0.15f }, ID::in_attack_button, _texture_holder));

	_armies.push_back(std::vector<std::shared_ptr<Unit>>());

	//полный приздец
	_add_unit(Swordsman::MySwordsman({300, 650}, _texture_holder));
	_army_count += Swordsman::places_requres;
	_controlled_unit = _armies[0][0];

	for (int i = 0; i < total_defend_places; i++)
	{
		_defend_places.insert({ i, { defendline_x - (i / 5) * row_width, Y_MAP_MAX - 30 - (i % max_solders_in_row) * (Y_MAP_MAX - Y_MAP_MIN - 50) / max_solders_in_row } });
		_enemy_defend_places.insert({ total_defend_places - i, {enemy_defendline_x - (i / 5) * row_width, Y_MAP_MAX - 30 - (i % max_solders_in_row) * (Y_MAP_MAX - Y_MAP_MIN - 50) / max_solders_in_row }});
	}
	for (const auto goldmine_position : goldmine_positions)
		_add_gold_mine(goldmine_position, _texture_holder);

}

int Game::run()
{
	_main_window.setFramerateLimit(120);
	while (_main_window.isOpen())
	{
		sf::Time deltatime = this->_clock.restart();
		_process_events();
		_handle_inputs(deltatime);
		_process_internal_actions(deltatime);
		
		this->_draw();
		_main_window.display();
	}
	return 0;
}

void Game::_add_unit(std::shared_ptr<Unit> unit)
{
	_armies[0].push_back(unit);
	//_army_count += unit->get_places_requres();
	unit->set_target(_current_target);
}

void Game::_add_unit(Unit* unit)
{
	_armies[0].emplace_back(unit);
	//_army_count += unit->get_places_requres();
	unit->set_target(_current_target);
}

int Game::_move_camera(int step)
{
	int prev_camera_position = _camera_position;
	_camera_position += step;
	if (_camera_position < min_camera_position)
		_camera_position = -min_camera_position;
	else if (_camera_position > max_camera_position)
		_camera_position = max_camera_position;
	int actual_step = _camera_position - prev_camera_position;
	
	_background_sprite.setTextureRect({ _camera_position, 0, 2100, 1050 });
	for (const auto army : _armies)
		for (const auto unit : army)
			unit->move_sprite({ -actual_step, 0 });

	for (const auto enemy : _enemy_army)
		enemy->move_sprite({ -actual_step, 0 });

	_camera_position_text.setString("x: " + std::to_string(_camera_position));
	return actual_step;
}

void Game::_add_enemy_unit(std::shared_ptr<Unit> unit)
{
	unit->set_target(_current_enemy_target);
	_enemy_army.push_back(unit);
	_enemy_army_count += unit.get()->get_places_requres();
}

void Game::_add_gold_mine(sf::Vector2f position, TextureHolder& holder)
{
	_gold_mines.emplace_back(new GoldMine(position, holder));
}

Button::Button(sf::Vector2f position, sf::Vector2f scale, ID id, TextureHolder& holder)
{
	_sprite.setTexture(holder.getTexture(id));
	_sprite.setPosition(position);
	_sprite.setScale(scale);
}

const sf::Sprite& Button::get_sprite() const
{
	return _sprite;
}

void Button::draw(sf::RenderWindow& window) const
{
	window.draw(_sprite);
}

void Button::press()
{
	_pressed = true;
}

bool Button::is_pressed()
{
	bool temp = _pressed;
	_pressed = false;
	return temp;
}

UnitBuyButton::UnitBuyButton(int unit_cost, int wait_time, sf::Vector2f position, sf::Vector2f scale, ID id, TextureHolder& holder, sf::Font& font)
	: Button(position, scale, id, holder), _unit_cost(unit_cost), _wait_time(wait_time)
{
	_timebar.setPosition({ position.x + 3, position.y + 100 });
	_timebar.setFillColor(sf::Color::Cyan);
	_timebar.setSize({ 0, 0 });

	_count_text.setFont(font);
	_count_text.setFillColor(sf::Color::Black);
	_count_text.setPosition({ position.x + 10, position.y + 50 });

	_gold_icon.setTexture(holder.getTexture(ID::gold));
	_gold_icon.setScale({ 0.04, 0.04 });
	_gold_icon.setPosition({ position.x + 5, position.y + 10 });

	_cost_text.setFont(font);
	_cost_text.setString(std::to_string(unit_cost).c_str());
	_cost_text.setFillColor(sf::Color::Black);
	_cost_text.setPosition({ position.x + 40, position.y + 10 });
	_cost_text.setCharacterSize(20);
}

void UnitBuyButton::draw(sf::RenderWindow& window) const
{
	window.draw(_sprite);
	window.draw(_timebar);
	window.draw(_gold_icon);
	window.draw(_cost_text);
	if(_remaining_time != 0)
		window.draw(_count_text);
}

void UnitBuyButton::press()
{
	_remaining_time += _wait_time;
	this->process_button(1);
}

int UnitBuyButton::get_unit_cost() const
{
	return _unit_cost;
}

void UnitBuyButton::process_button(int elapsed_time)
{
	if (_remaining_time >= elapsed_time)
		_remaining_time -= elapsed_time;
	else
		_remaining_time = 0;

	std::string temp_str = "x" + std::to_string((int)ceil(static_cast<float>(_remaining_time) / _wait_time));
	_count_text.setString(temp_str.c_str());

	int remainig_time_for_current_unit = _remaining_time % _wait_time;
	_timebar.setSize({ _bar_size.x * remainig_time_for_current_unit / _wait_time, _bar_size.y });
}

bool random(float probability)
{
	if (probability <= 0)
		return false;
	else if (probability >= 1)
		return true;

	int p = 1 + rand() % 100000;
	return p <= static_cast<int>(probability * 100000);
}
