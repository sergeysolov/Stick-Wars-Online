#include "Units.h"

sf::Vector2f HEALTHBAR_SHIFT = { -32, 50 };

Unit::Unit(TextureHolder& holder, ID id, sf::Vector2f spawnpoint, float health, float _armor, float speed, float damage, float damage_speed, float attack_distance, AnimationParams animation_params) :
	MapObject(spawnpoint, holder, id, animation_params), health_(health), max_health_(health), armor_(_armor), speed_(speed), damage_(damage), damage_speed_(damage_speed), attack_distance_(attack_distance)
{
	health_bar_.setPosition({ x_ + HEALTHBAR_SHIFT.x, y_ + HEALTHBAR_SHIFT.y});
	health_bar_.setSize({ max_healthbar_size, 3 });
	health_bar_.setFillColor(sf::Color::Magenta);
}

void Unit::show_animation()
{
	if (cumulative_time_ > animation_step)
	{
		cumulative_time_ -= animation_step;
		(current_frame_ += 1) %= animation_params_.total_frames;

		if (current_frame_ == 0)
			cumulative_time_ = 0;

		int y_shift = 0;

		if (animation_type_ == AnimatonType::attack_animation)
		{
			y_shift = animation_params_.frame_height;
			if (current_frame_ == 7)
				do_damage_ = true;
		}

		if (animation_type_ == AnimatonType::die_animation)
			y_shift = animation_params_.frame_height * 2;
		
		sprite_.setTextureRect({ animation_params_.init_position.x + animation_params_.frame_width * current_frame_, animation_params_.init_position.y + y_shift, animation_params_.frame_width, animation_params_.frame_height });
	}
}

void Unit::couse_damage(float _damage)
{
	health_ -= _damage / armor_;
	update_health_bar();
	if (health_ <= 0)
		kill();
}

bool Unit::is_alive() const
{
	return health_ > 0;
}

float Unit::get_speed() const
{
	return speed_;
}

int Unit::get_spawn_time() const
{
	return spawn_time_;
}

void Unit::update_health_bar()
{
	float _health_bar_size = (health_ / max_health_) * max_healthbar_size;
	if(health_ < 0)
		_health_bar_size = 0;
	health_bar_.setSize({ _health_bar_size, health_bar_.getSize().y });
}

void Unit::set_y_scale()
{
	float scale_factor = (a * sprite_.getPosition().y + b);
	sprite_.setScale({ scale_factor * prev_direction_ * animation_params_.scale.x, scale_factor * animation_params_.scale.y });
}

Target Unit::get_target() const
{
	return target_;
}

bool Unit::animation_complete()
{
	if (current_frame_ == animation_params_.total_frames - 1)
	{
		if (animation_type_ == AnimatonType::die_animation)
		{
			health_ = 0;
			return true;
		}
		if (animation_type_ == AnimatonType::attack_animation)
		{
			animation_type_ = AnimatonType::no_animation;
			return true;
		}
	}
	if (cumulative_time_ > 0)
		return false;
	return true;
}

std::pair<int, sf::Vector2f> Unit::get_stand_place() const
{
	return stand_place_;
}

std::pair<int, sf::Vector2f> Unit::extract_stand_place()
{
	auto temp = stand_place_;
	stand_place_ = { 0,  { 1E+15f, 1E+15f } };
	return temp;
}

void Unit::set_stand_place(std::map<int, sf::Vector2f>& places)
{
	stand_place_ = *places.begin();
	places.erase(places.begin());
}

void Unit::set_target(Target target)
{
	target_ = target;
}

int Unit::get_places_requres() const
{
	return 0;
}

int Unit::get_direction() const
{
	return prev_direction_;
}

float Unit::get_attack_distance() const
{
	return attack_distance_;
}

float Unit::get_damage() const
{
	return damage_;
}

void Unit::set_screen_place(int camera_position)
{
	sprite_.setPosition({ x_ - camera_position, y_ });
	health_bar_.setPosition({ x_ - camera_position + HEALTHBAR_SHIFT.x, y_ + HEALTHBAR_SHIFT.y});
}




void Unit::move_sprite(sf::Vector2i vc)
{
	sprite_.move((float)vc.x, (float)vc.y );
	health_bar_.move((float)vc.x, (float)vc.y);
}

void Unit::move(sf::Vector2i direction, sf::Time time)
{
	float x_offset = direction.x * time.asMilliseconds() * speed_;
	float y_offset = direction.y * time.asMilliseconds() * vertical_speed_;
	const float new_x = x_offset + x_;
	const float new_y = y_offset + y_;

	if (new_x > x_map_min and new_x < x_map_max and new_y > y_map_min and new_y < y_map_max)
	{
		x_ += x_offset;
		y_ += y_offset;
		sprite_.move({ x_offset, y_offset });
		health_bar_.move({ x_offset, y_offset });
	}

	if (direction.x != 0 and direction.x != prev_direction_)
	{
		prev_direction_ = direction.x;
		sprite_.scale(-1, 1);
	}
	set_y_scale();

	if (animation_type_ != AnimatonType::walk_animation)
		current_frame_ = 0;
	animation_type_ = AnimatonType::walk_animation;
	cumulative_time_++;
}

void Unit::kill()
{
	//_health = 0;
	//_update_health_bar();
	if (animation_type_ != AnimatonType::die_animation)
	{
		current_frame_ = 0;
	}
	cumulative_time_++;
	animation_type_ = AnimatonType::die_animation;
	killed_ = true;
}

bool Unit::is_killed()
{
	const bool temp = killed_;
	killed_ = false;
	return temp;
}

bool Unit::can_do_damage()
{
	const bool temp = do_damage_;
	do_damage_ = false;
	return temp;
}

void Unit::draw(sf::RenderWindow& window) const
{
	window.draw(sprite_);
	window.draw(health_bar_);
}

void Unit::commit_attack()
{
	if (animation_type_ != AnimatonType::attack_animation)
	{
		current_frame_ = 0;
	}
	cumulative_time_++;
	animation_type_ = AnimatonType::attack_animation;
}

Miner::Miner(sf::Vector2f spawnpoint, TextureHolder& holder, ID id) : 
	Unit(holder, id, spawnpoint, health_=100, armor_=1, speed_=0.2f, damage_=5, damage_speed_=1.0f, attack_distance_=150,
		{ {-300, 2}, 700, 1280, 13, {-0.4, 0.4 } })
{
	//_animation_params = { {0, 0}, 402, 246, 12, { 0.4, 0.4 } };
	
	spawn_time_ = miner_wait_time;
}

int Miner::get_places_requres() const
{
	return places_requres;
}

Unit* Miner::MakeMiner(sf::Vector2f spawnpoint, TextureHolder& holder)
{
	return new Miner(spawnpoint, holder, ID::miner);
}

Unit* Miner::EnemyMiner(sf::Vector2f spawnpoint, TextureHolder& holder)
{
	return new Miner(spawnpoint, holder, ID::miner_enemy);
}

Swordsman::Swordsman(sf::Vector2f spawnpoint, TextureHolder& holder, ID id)
	: Unit(holder, id, spawnpoint, health_=300, armor_=1, speed_=0.3f, damage_=30, damage_speed_=3.0f, attack_distance_=170,
		{ {-300, 20}, 700, 1280, 13, {-0.4, 0.4} })
{
	spawn_time_ = swardsman_wait_time;
}

int Swordsman::get_places_requres() const
{
	return places_requres;
}

Unit* Swordsman::MakeSwordsman(sf::Vector2f spawnpoint, TextureHolder& holder)
{
	return new Swordsman(spawnpoint, holder, ID::swordsman);
}

Unit* Swordsman::EnemySwordsman(sf::Vector2f spawnpoint, TextureHolder& holder)
{
	return new Swordsman(spawnpoint, holder, ID::swordsman_enemy);
}

