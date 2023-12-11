#include "Units.h"

Unit::Unit(TextureHolder& holder, ID id, sf::Vector2f spawnpoint, float health, float speed, float damage, float attack_distance, int spawn_time, AnimationParams animation_params) :
	MapObject(spawnpoint, holder, id, animation_params), health_(health), max_health_(health), speed_(speed), damage_(damage),
	attack_distance_(attack_distance), spawn_time_(spawn_time), health_bar_(max_health_, health_, spawnpoint)
{	}

void Unit::show_animation(const int delta_time)
{
	if(not animation_complete())
	{
		cumulative_time_ += delta_time;
		if (cumulative_time_ > animation_step)
		{
			cumulative_time_ -= animation_step;
			(current_frame_ += 1) %= animation_params_.total_frames;

			if (current_frame_ == 0)
				cumulative_time_ = 0;

			int y_shift = 0;

			if (animation_type_ == attack_animation)
			{
				y_shift = animation_params_.frame_height;
				if (current_frame_ == 7)
					do_damage_flag_ = true;
			}

			if (animation_type_ == die_animation)
				y_shift = animation_params_.frame_height * 2;

			sprite_.setTextureRect({ animation_params_.init_position.x + animation_params_.frame_width * current_frame_, animation_params_.init_position.y + y_shift, animation_params_.frame_width, animation_params_.frame_height });
		}
	}
}

void Unit::cause_damage(const float damage)
{
	health_ = std::max(health_ - damage, 0.f);
	health_bar_.update();
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

void Unit::set_y_scale()
{
	const float scale_factor = (a * sprite_.getPosition().y + b);
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
		if (animation_type_ == die_animation)
			return true;
		if (animation_type_ == attack_animation)
		{
			animation_type_ = no_animation;
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

void Unit::set_target(const Target target)
{
	target_ = target;
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

void Unit::set_screen_place(const float camera_position)
{
	sprite_.setPosition({ x_ - camera_position, y_ });
	health_bar_.set_position ({ x_ - camera_position, y_});
}


void Unit::move_sprite(const sf::Vector2f offset)
{
	MapObject::move_sprite(offset);
	health_bar_.move({ static_cast<float>(offset.x), static_cast<float>(offset.y) });
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

	if (animation_type_ != walk_animation)
		current_frame_ = 0;
	animation_type_ = walk_animation;
	cumulative_time_++;
}

void Unit::kill()
{
	if (animation_type_ != die_animation)
	{
		current_frame_ = 0;
	}
	cumulative_time_++;
	animation_type_ = die_animation;
	dead_ = true;
}

bool Unit::was_killed()
{
	const bool temp = dead_;
	dead_ = false;
	return temp;
}

bool Unit::can_do_damage()
{
	const bool temp = do_damage_flag_;
	do_damage_flag_ = false;
	return temp;
}

void Unit::draw(sf::RenderWindow& window) const
{
	window.draw(sprite_);
	health_bar_.draw(window);
}

void Unit::commit_attack()
{
	if (animation_type_ != attack_animation)
	{
		current_frame_ = 0;
	}
	cumulative_time_++;
	animation_type_ = attack_animation;
}

Miner::Miner(sf::Vector2f spawnpoint, TextureHolder& holder, ID id)
	: Unit(holder, id, spawnpoint, max_health, speed, damage, attack_distance, wait_time, animation_params)
{	}

int Miner::get_places_requres() const
{
	return places_requres;
}

ID Miner::get_id() const
{
	return texture_id;
}

Swordsman::Swordsman(sf::Vector2f spawnpoint, TextureHolder& holder, ID id)
	: Unit(holder, id, spawnpoint, max_health, speed, damage, attack_distance, wait_time, animation_params)
{	}

int Swordsman::get_places_requres() const
{
	return places_requres;
}

ID Swordsman::get_id() const
{
	return texture_id;
}

