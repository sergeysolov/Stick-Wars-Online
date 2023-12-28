#include "Units.h"

Unit::Unit(texture_ID id, sf::Vector2f spawn_point, float health, const AnimationParams& animation_params) :
	MapObject(spawn_point, id, animation_params), health_(health), health_bar_(health, health_, spawn_point, Bar<float>::unit_health_bar_size, Bar<float>::unit_health_bar_shift, Bar<float>::health_bar_color)
{
	
}

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

			set_animation_frame();
		}
	}
}

void Unit::set_animation_frame(const bool is_play_hit_sound)
{
	int y_shift = 0; // if walk_animation

	if (animation_type_ == attack_animation)
	{
		if(is_play_hit_sound and current_frame_ == 2 and get_id() > 0)
			play_hit_sound();

		y_shift = animation_params_.frame_height;
		if (current_frame_ == 7)
			do_damage_flag_ = true;
	}

	if (animation_type_ == die_animation)
		y_shift = animation_params_.frame_height * 2;

	sprite_.setTextureRect({ animation_params_.init_position.x + animation_params_.frame_width * current_frame_, animation_params_.init_position.y + y_shift, animation_params_.frame_width, animation_params_.frame_height });
}


void Unit::cause_damage(const float damage, const int direction)
{
	health_ = std::clamp(health_ - damage, 0.f, get_max_health());
	push(direction);
	health_bar_.update();
	if (damage > 0)
		play_damage_sound();
	if (health_ <= 0)
		kill();
}

bool Unit::is_alive() const
{
	return health_ > 0;
}

sf::Vector2f Unit::get_speed() const
{
	return speed_;
}

void Unit::set_y_scale()
{
	const float scale_factor = ( scale_y_param_a * sprite_.getPosition().y + scale_y_param_b);
	sprite_.setScale({ scale_factor * prev_direction_ * animation_params_.scale.x, scale_factor * animation_params_.scale.y });
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
			return false;
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

void Unit::write_to_packet(sf::Packet& packet) const
{
	MapObject::write_to_packet(packet);
	packet << health_ << speed_.x << speed_.y << animation_type_ << was_move_.first << was_move_.second << prev_direction_
		<< dead_ << do_damage_flag_;
}

void Unit::update_from_packet(sf::Packet& packet)
{
	const auto prev_frame = current_frame_;
	MapObject::update_from_packet(packet);
	const float prev_health = health_;

	int animation_type;
	packet >> health_ >> speed_.x >> speed_.y >> animation_type >> was_move_.first >> was_move_.second >> prev_direction_
		>> dead_ >> do_damage_flag_;
	animation_type_ = static_cast<AnimationType>(animation_type);

	if (prev_health > health_)
		play_damage_sound();
	if (get_id() > 0 and prev_frame == 1 and current_frame_ == 2 and animation_type_ == attack_animation)
		play_hit_sound();

	cause_damage(0, 0);
	set_animation_frame(false);
}

int Unit::get_direction() const
{
	return prev_direction_;
}

void Unit::set_screen_place(const float camera_position)
{
	MapObject::set_screen_place(camera_position);
	health_bar_.set_position({ x_ - camera_position, y_ });
}

void Unit::process_move(const sf::Time time)
{
	const float dx = time.asMilliseconds() * speed_.x;
	const float dy = time.asMilliseconds() * speed_.y;

	constexpr float min_shift = 0.05f;

	if(abs(dx) > min_shift or abs(dy) > min_shift)
	{
		x_ = std::clamp(x_ + dx, x_map_min, x_map_max);
		y_ = std::clamp(y_ + dy, y_map_min, y_map_max);

		sprite_.setPosition({ x_, y_ });
		health_bar_.set_position({ x_, y_ });

		set_y_scale();

		if (animation_type_ != walk_animation)
			current_frame_ = 0;
		animation_type_ = walk_animation;
		cumulative_time_++;
	}

	constexpr float floor_speed_val = 0.1f;

	if(not was_move_.first)
	{
		if (abs(speed_.x) < floor_speed_val)
			speed_.x = 0.0f;
		else
			speed_.x -= speed_.x / abs(speed_.x) * acceleration * time.asMilliseconds();
	}

	if(not was_move_.second)
	{
		if (abs(speed_.y) < floor_speed_val)
			speed_.y = 0.0f;
		else
			speed_.y -= speed_.y / abs(speed_.y) * acceleration * time.asMilliseconds();
	}
	was_move_ = {false, false};
}

void Unit::move(const sf::Vector2i direction, const sf::Time time)
{
	if(animation_type_ == attack_animation and current_frame_ < 8) // Can not move while attack
		return;

	was_move_ = { direction.x != 0, direction.y != 0 };

	speed_.x = std::clamp(speed_.x + time.asMilliseconds() * acceleration * direction.x, -get_max_speed().x, get_max_speed().x);
	speed_.y = std::clamp(speed_.y + time.asMilliseconds() * acceleration * direction.y, -get_max_speed().y, get_max_speed().y);

	if (direction.x != 0 and direction.x != prev_direction_)
	{
		prev_direction_ = direction.x;
		sprite_.scale(-1, 1);
	}
}

void Unit::kill()
{
	if (animation_type_ != die_animation)
		current_frame_ = 0;
	animation_type_ = die_animation;
	cumulative_time_++;
	dead_ = true;

	play_kill_sound();
}

void Unit::push(const int direction)
{
	speed_.x = std::clamp(speed_.x + direction * 0.3f, -0.8f, 0.8f);
}

void Unit::play_kill_sound()
{
	static constexpr int total_kill_sounds = 3;
	static std::vector<sf::Sound> kill_sounds;
	if (kill_sounds.empty())
	{
		for (int i = 0; i < total_kill_sounds; i++)
		{
			kill_sounds.emplace_back();
			kill_sounds.back().setBuffer(sound_buffers_holder.get_sound_buffer(sward_kill));
			kill_sounds.back().setVolume(sounds_volume);
		}
	}
	static int idx = 0;
	idx = (idx + 1) % total_kill_sounds;
	kill_sounds[idx].play();
}

void Unit::play_damage_sound()
{
	static constexpr int total_damage_sounds = 8;
	static std::vector<sf::Sound> damage_sounds;
	if (damage_sounds.empty())
	{
		for (int i = 0; i < total_damage_sounds; i++)
		{
			damage_sounds.emplace_back();
			damage_sounds.back().setBuffer(sound_buffers_holder.get_sound_buffer(sward_damage));
			damage_sounds.back().setVolume(sounds_volume);
		}
	}
	static int idx = 0;
	idx = (idx + 1) % total_damage_sounds;
	damage_sounds[idx].play();
}

void Unit::play_hit_sound()
{
	static constexpr int total_hit_sounds = 8;
	static std::vector<sf::Sound> hit_sounds;
	if (hit_sounds.empty())
	{
		for (int i = 0; i < total_hit_sounds; i++)
		{
			hit_sounds.emplace_back();
			hit_sounds.back().setBuffer(sound_buffers_holder.get_sound_buffer(sward_hit));
			hit_sounds.back().setVolume(sounds_volume);
		}
	}
	static int idx = 0;
	idx = (idx + 1) % total_hit_sounds;
	if(hit_sounds[idx].getStatus() != sf::Sound::Playing)
		hit_sounds[idx].play();
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

void Unit::draw(DrawQueue& queue) const
{
	if (is_alive())
	{
		queue.emplace(alive_units, &sprite_);
		if (abs(health_ - get_max_health()) > 1e-5)
			health_bar_.draw(queue);
	}
	else
		queue.emplace(dead_units,&sprite_);
}

void Unit::commit_attack()
{
	if (animation_type_ != attack_animation)
	{
		current_frame_ = 0;
	}
	animation_type_ = attack_animation;

	cumulative_time_++;
}

Miner::Miner(const sf::Vector2f spawn_point, const texture_ID texture_id)
	: Unit(texture_id, spawn_point, max_health, animation_params),
	gold_count_bar_(gold_bag_capacity, gold_count_in_bag_, spawn_point, Bar<int>::unit_health_bar_size, Bar<int>::miner_gold_count_bar_shift, Bar<int>::miner_gold_bar_color)
{
	
}

void Miner::draw(DrawQueue& queue) const
{
	Unit::draw(queue);
	if(is_alive())
		gold_count_bar_.draw(queue);
}

void Miner::set_screen_place(const float camera_position)
{
	Unit::set_screen_place(camera_position);
	gold_count_bar_.set_position({ x_ - camera_position, y_ });
}

void Miner::move(const sf::Vector2i direction, const sf::Time time)
{
	Unit::move(direction, time);
	gold_count_bar_.set_position({ x_, y_ });
}

void Miner::fill_bag(const int gold_count)
{
	gold_count_in_bag_ = std::min(gold_count_in_bag_ + gold_count, gold_bag_capacity);
	gold_count_bar_.update();
}

bool Miner::is_bag_filled() const
{
	return gold_bag_capacity == gold_count_in_bag_;
}

int Miner::flush_bag()
{
	const int gold_count = gold_count_in_bag_;
	gold_count_in_bag_ = 0;
	gold_count_bar_.update();
	return gold_count;
}


int Miner::get_places_requires() const
{
	return places_requires;
}

float Miner::get_max_health() const
{
	return max_health;
}

sf::Vector2f Miner::get_max_speed() const
{
	return max_speed;
}

float Miner::get_damage() const
{
	return damage;
}

float Miner::get_attack_distance() const
{
	return attack_distance;
}

int Miner::get_wait_time() const
{
	return wait_time;
}

int Miner::get_cost() const
{
	return cost;
}

void Miner::write_to_packet(sf::Packet& packet) const
{
	packet << id << gold_count_in_bag_;
	Unit::write_to_packet(packet);
}

void Miner::update_from_packet(sf::Packet& packet)
{
	packet >> gold_count_in_bag_;
	gold_count_bar_.update();
	Unit::update_from_packet(packet);
}

int Miner::get_id() const
{
	return id;
}

Swordsman::Swordsman(const sf::Vector2f spawn_point, const texture_ID texture_id)
	: Unit(texture_id, spawn_point, max_health, animation_params)
{
}

void Swordsman::commit_attack()
{
	Unit::commit_attack();
}

int Swordsman::get_places_requires() const
{
	return places_requires;
}

float Swordsman::get_max_health() const
{
	return max_health;
}

sf::Vector2f Swordsman::get_max_speed() const
{
	return max_speed;
}

float Swordsman::get_damage() const
{
	return damage;
}

float Swordsman::get_attack_distance() const
{
	return attack_distance;
}

int Swordsman::get_wait_time() const
{
	return wait_time;
}

int Swordsman::get_cost() const
{
	return cost;
}

void Swordsman::write_to_packet(sf::Packet& packet) const
{
	packet << id;
	Unit::write_to_packet(packet);
}

int Swordsman::get_id() const
{
	return id;
}


