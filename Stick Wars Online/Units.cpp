#include "Units.h"

sf::Vector2f HEALTHBAR_SHIFT = { -32, 50 };

Unit::Unit(TextureHolder& holder, ID id, sf::Vector2f spawnpoint, float _health, float _armor, float _speed, float _damage, float _damage_speed, float _attack_distance, AnimationParams _animation_params) :
	MapObject(spawnpoint, holder, id, _animation_params), _health(_health), _max_health(_health), _armor(_armor), _speed(_speed), _damage(_damage), _damage_speed(_damage_speed), _attack_distance(_attack_distance)
{
	_health_bar.setPosition({ _x + HEALTHBAR_SHIFT.x, _y + HEALTHBAR_SHIFT.y});
	_health_bar.setSize({ _max_healthbar_size, 3 });
	_health_bar.setFillColor(sf::Color::Magenta);
}

void Unit::show_animation()
{
	if (_cumulative_time > ANIMATION_STEP)
	{
		_cumulative_time -= ANIMATION_STEP;
		(_current_frame += 1) %= _animation_params.total_frames;

		if (_current_frame == 0)
			_cumulative_time = 0;

		int y_shift = 0;

		if (_animation_type == AnimatonType::attack)
		{
			y_shift = _animation_params.frame_height;
			if (_current_frame == 7)
				_do_damage = true;
		}

		if (_animation_type == AnimatonType::die)
			y_shift = _animation_params.frame_height * 2;
		
		_sprite.setTextureRect({ _animation_params.init_position.x + _animation_params.frame_width * _current_frame, _animation_params.init_position.y + y_shift, _animation_params.frame_width, _animation_params.frame_height });
	}
}

void Unit::couse_damage(float _damage)
{
	_health -= _damage / _armor;
	_update_health_bar();
	if (_health <= 0)
		kill();
}

bool Unit::is_alive() const
{
	return _health > 0;
}

float Unit::get_speed() const
{
	return _speed;
}

int Unit::get_spawn_time() const
{
	return _spawn_time;
}

void Unit::_update_health_bar()
{
	float _health_bar_size = (_health / _max_health) * _max_healthbar_size;
	if(_health < 0)
		_health_bar_size = 0;
	_health_bar.setSize({ _health_bar_size, _health_bar.getSize().y });
}

void Unit::_set_y_scale()
{
	float scale_factor = (a * _sprite.getPosition().y + b);
	_sprite.setScale({ scale_factor * _prev_direction * _animation_params.scale.x, scale_factor * _animation_params.scale.y });
}

Target Unit::get_target() const
{
	return _target;
}

bool Unit::animation_complete()
{
	if (_current_frame == _animation_params.total_frames - 1)
	{
		if (_animation_type == AnimatonType::die)
		{
			_health = 0;
			return true;
		}
		if (_animation_type == AnimatonType::attack)
		{
			_animation_type = AnimatonType::none;
			return true;
		}
	}
	if (_cumulative_time > 0)
		return false;
	return true;
}

std::pair<int, sf::Vector2f> Unit::get_stand_place() const
{
	return _stand_place;
}

std::pair<int, sf::Vector2f> Unit::extract_stand_place()
{
	auto temp = _stand_place;
	_stand_place = { 0,  { 1E+15f, 1E+15f } };
	return temp;
}

void Unit::set_stand_place(std::map<int, sf::Vector2f>& places)
{
	_stand_place = *places.begin();
	places.erase(places.begin());
}

void Unit::set_target(Target target)
{
	_target = target;
}

int Unit::get_places_requres() const
{
	return 0;
}

int Unit::get_direction() const
{
	return _prev_direction;
}

float Unit::get_attack_distance() const
{
	return _attack_distance;
}

float Unit::get_damage() const
{
	return _damage;
}

void Unit::set_screen_place(int camera_position)
{
	_sprite.setPosition({ _x - camera_position, _y });
	_health_bar.setPosition({ _x - camera_position + HEALTHBAR_SHIFT.x, _y + HEALTHBAR_SHIFT.y});
}




void Unit::move_sprite(sf::Vector2i vc)
{
	_sprite.move((float)vc.x, (float)vc.y );
	_health_bar.move((float)vc.x, (float)vc.y);
}

void Unit::move(sf::Vector2i direction, sf::Time time)
{
	float x_offset = direction.x * time.asMilliseconds() * _speed;
	float y_offset = direction.y * time.asMilliseconds() * _vertical_speed;
	float new_x = x_offset + _x;
	float new_y = y_offset + _y;

	if (new_x > X_MAP_MIN and new_x < X_MAP_MAX and new_y > Y_MAP_MIN and new_y < Y_MAP_MAX)
	{
		_x += x_offset;
		_y += y_offset;
		_sprite.move({ x_offset, y_offset });
		_health_bar.move({ x_offset, y_offset });
	}

	if (direction.x != 0 and direction.x != _prev_direction)
	{
		_prev_direction = direction.x;
		_sprite.scale(-1, 1);
	}
	_set_y_scale();

	if (_animation_type != AnimatonType::walk)
		_current_frame = 0;
	_animation_type = AnimatonType::walk;
	_cumulative_time++;
}

void Unit::kill()
{
	//_health = 0;
	//_update_health_bar();
	if (_animation_type != AnimatonType::die)
	{
		_current_frame = 0;
	}
	_cumulative_time++;
	_animation_type = AnimatonType::die;
	_killed = true;
}

bool Unit::is_killed()
{
	bool temp = _killed;
	_killed = false;
	return temp;
}

bool Unit::can_do_damage()
{
	bool temp = _do_damage;
	_do_damage = false;
	return temp;
}

void Unit::draw(sf::RenderWindow& window) const
{
	window.draw(_sprite);
	window.draw(_health_bar);
}

void Unit::commit_attack()
{
	if (_animation_type != AnimatonType::attack)
	{
		_current_frame = 0;
	}
	_cumulative_time++;
	_animation_type = AnimatonType::attack;
}

Miner::Miner(sf::Vector2f spawnpoint, TextureHolder& holder, ID id) : 
	Unit(holder, id, spawnpoint, _health=100, _armor=1, _speed=0.2f, _damage=5, _damage_speed=1.0f, _attack_distance=150,
		{ {-300, 2}, 700, 1280, 13, {-0.4, 0.4 } })
{
	//_animation_params = { {0, 0}, 402, 246, 12, { 0.4, 0.4 } };
	
	_spawn_time = MINER_WAIT_TIME;
}

int Miner::get_places_requres() const
{
	return places_requres;
}

Unit* Miner::MyMiner(sf::Vector2f spawnpoint, TextureHolder& holder)
{
	return new Miner(spawnpoint, holder, ID::miner);
}

Unit* Miner::EnemyMiner(sf::Vector2f spawnpoint, TextureHolder& holder)
{
	return new Miner(spawnpoint, holder, ID::miner_enemy);
}

Swordsman::Swordsman(sf::Vector2f spawnpoint, TextureHolder& holder, ID id)
	: Unit(holder, id, spawnpoint, _health=300, _armor=1, _speed=0.3f, _damage=30, _damage_speed=3.0f, _attack_distance=170,
		{ {-300, 20}, 700, 1280, 13, {-0.4, 0.4} })
{
	_spawn_time = SWARDSMAN_WAIT_TIME;
}

int Swordsman::get_places_requres() const
{
	return places_requres;
}

Unit* Swordsman::MySwordsman(sf::Vector2f spawnpoint, TextureHolder& holder)
{
	return new Swordsman(spawnpoint, holder, ID::swordsman);
}

Unit* Swordsman::EnemySwordsman(sf::Vector2f spawnpoint, TextureHolder& holder)
{
	return new Swordsman(spawnpoint, holder, ID::swordsman_enemy);
}

