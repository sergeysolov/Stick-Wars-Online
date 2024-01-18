#include "Units.h"

#include <functional>
#include <random>

#include "Player.h"

Unit::Unit(const texture_ID id, const sf::Vector2f spawn_point, const float health, const SpriteParams& animation_params) :
	MapObject(spawn_point, id, animation_params), health_(health), health_bar_(health, health_, spawn_point, Bar<float>::unit_bar_size, Bar<float>::unit_health_bar_offset, Bar<float>::health_bar_color)
{
	stun_stars_sprite_.setTexture(texture_holder.get_texture(stun_stars));
	stun_stars_sprite_.setScale(stun_sprite_scale);
}

void Unit::show_animation(const int delta_time)
{
	if (not animation_complete())
	{
		const SpriteParams::AnimationParams params = sprite_params_.animations[std::max(walk_animation, animation_type_)];

		cumulative_time_ += delta_time;
		if (cumulative_time_ > params.time_frame)
		{
			cumulative_time_ -= params.time_frame;
			(current_frame_ += 1) %= params.total_frames;

			if (current_frame_ == 0)
				cumulative_time_ = 0;

			set_animation_frame();
		}
	}
}

void Unit::set_animation_frame(const bool is_play_hit_sound)
{
	if(animation_type_ == walk_animation)
	{
		if ((abs(speed_.x) > 0.05f or abs(speed_.y) > 0.05f) and current_frame_ <= 1)
			current_frame_ = 2;
	}
	else if (animation_type_ == attack_animation)
	{
		if(current_frame_ == get_damage_frame())
			do_damage_flag_ = true;

		if (is_play_hit_sound)
			play_hit_sound();
	}
	else if(animation_type_ == second_attack_animation)
	{
		if (current_frame_ == get_second_attack_damage_frame())
			do_damage_flag_ = true;
	}

	const int y_shift = std::max(walk_animation, animation_type_) * sprite_params_.frame_height;

	sprite_.setTextureRect({ sprite_params_.init_position.x + sprite_params_.frame_width * current_frame_, sprite_params_.init_position.y + y_shift, sprite_params_.frame_width, sprite_params_.frame_height });
}


std::pair<float, Unit::DamageType> Unit::cause_damage(const float damage, const int direction, const int stun_time)
{
	if (not can_be_damaged())
		return { 0.f, no_damage };

	const float prev_health = health_;
	health_ = std::clamp(health_ - damage, 0.f, get_max_health());
	stun_time_left_ += stun_time;
	push(direction);
	health_bar_.update();

	const float actual_damage = prev_health - health_;
	if (abs(actual_damage) > 1e-5)
		shared_effects_manager.add_effect(std::make_unique<DropDamageEffect>(sf::Vector2f{ x_, y_ }, actual_damage));

	DamageType damage_type = damage > 0 ? is_damage : no_damage;
	if (health_ <= 1e-5)
	{
		kill();
		damage_type = is_kill;
	}
	return {actual_damage, damage_type};
}

bool Unit::is_alive() const
{
	return health_ > 0;
}

int Unit::get_splash_count() const
{
	return 1;
}

int Unit::get_stun_time() const
{
	return 0;
}

int Unit::get_second_attack_damage_frame() const
{
	return 0;
}

sf::Vector2f Unit::get_speed() const
{
	return speed_;
}

void Unit::set_y_scale()
{
	const float scale_factor = ( scale_y_param_a * sprite_.getPosition().y + scale_y_param_b);
	sprite_.setScale({ scale_factor * prev_direction_ * sprite_params_.scale.x, scale_factor * sprite_params_.scale.y });
}

bool Unit::animation_complete()
{
	if (animation_type_ == no_animation)
		return true;
	if (current_frame_ == sprite_params_.animations[animation_type_].total_frames - 1)
	{
		if (animation_type_ == die_animation)
			return true;
		if (animation_type_ == attack_animation or animation_type_ == second_attack_animation)
		{
			animation_type_ = no_animation;
			return false;
		}
	}
	if (cumulative_time_ != 0)
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
	packet << health_ << speed_.x << stun_time_left_ << animation_type_ << prev_direction_;// << was_move_.first;
}

void Unit::update_from_packet(sf::Packet& packet)
{
	MapObject::update_from_packet(packet);

	int animation_type;
	packet >> health_ >> speed_.x >> stun_time_left_ >> animation_type >> prev_direction_;// >> was_move_.first;
	animation_type_ = static_cast<AnimationType>(animation_type);

	cause_damage(0, 0, 0);
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
	stun_stars_sprite_.setPosition({ x_ - camera_position + stun_sprite_offset.x, y_ + stun_sprite_offset.y});
}

void Unit::process(const sf::Time time)
{
	stun_time_left_ = std::max(stun_time_left_ - time.asMilliseconds(), 0);

	const float dx = time.asMilliseconds() * speed_.x;
	const float dy = time.asMilliseconds() * speed_.y;

	constexpr float min_shift = 0.05f;

	if(abs(dx) > min_shift or abs(dy) > min_shift)
	{
		float left_wall = Army::escape_line, right_wall = Army::enemy_escape_line;

		if(try_escape)
		{
			left_wall = x_map_min;
			right_wall = x_map_max;
		}

		x_ += dx;
		if(x_ < left_wall)
		{
			x_ = left_wall;
			speed_.x = 0;
		}
		else if(x_ > right_wall)
		{
			x_ = right_wall;
			speed_.x = 0;
		}

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

	try_escape = x_ < Army::escape_line or x_ > Army::enemy_escape_line;
}

void Unit::move(const sf::Vector2i direction, const sf::Time time)
{
	if((animation_type_ == attack_animation and current_frame_ < get_damage_frame()) or (animation_type_ == second_attack_animation and current_frame_ < get_second_attack_damage_frame()) or stun_time_left_ > 0) // Can not move while attack or urder stun
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
}

void Unit::push(const int direction)
{
	speed_.x = std::clamp(speed_.x + direction * 0.3f, -0.8f, 0.8f);
}

void Unit::play_hit_sound() const
{

}

void Unit::play_second_attack_hit_sound() const
{
}

void Unit::play_damage_sound() const
{
}

void Unit::play_kill_sound() const
{
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

bool Unit::can_be_damaged() const
{
	return x_ > min_camera_position and x_ < max_camera_position + map_frame_width;
}

void Unit::draw(DrawQueue& queue) const
{
	const int layer = static_cast<int>((y_ - y_map_min) / 10.f);
	if (is_alive())
	{
		const auto priority = static_cast<DrawPriority>(alive_units + layer);
		queue.emplace(priority, &sprite_);
		if (abs(health_ - get_max_health()) > 1e-5)
			health_bar_.draw(queue);
		if (stun_time_left_ > 0)
			queue.emplace(attributes_layer_2, &stun_stars_sprite_);
	}
	else
	{
		const auto priority = static_cast<DrawPriority>(dead_units + layer);
		queue.emplace(priority, &sprite_);
	}
}

sf::FloatRect Unit::get_unit_rect() const
{
	const sf::FloatRect sprite_rect = sprite_.getGlobalBounds();
	const float rect_width = sprite_rect.width / 7;
	const float y_offset = sprite_rect.height / 6;
	const sf::Vector2f rect_position = sprite_.getPosition();
	return  { sf::Vector2f{rect_position.x - rect_width / 2, rect_position.y + y_offset}, {rect_width, sprite_rect.height - 2.5f * y_offset } };
}

void Unit::commit_attack()
{
	if(stun_time_left_ == 0)
	{
		if (animation_type_ != attack_animation)
		{
			current_frame_ = 0;
		}
		animation_type_ = attack_animation;

		cumulative_time_++;
	}
}

void Unit::commit_second_attack()
{
}

Miner::Miner(const sf::Vector2f spawn_point, const texture_ID texture_id)
	: Unit(texture_id, spawn_point, max_health, sprite_params),
	gold_count_bar_(gold_bag_capacity, gold_count_in_bag_, spawn_point, Bar<int>::unit_bar_size, Bar<int>::unit_second_attribute_bar_offset, Bar<int>::miner_gold_bar_color)
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
	if(gold_count > 0)
	{
		private_sound_manager.play_sound(money_sound);
		private_effect_manager.add_effect(std::make_unique<DropMoneyEffect>(sf::Vector2f{ x_, y_ }, gold_count));
	}
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

float Miner::get_damage(const std::shared_ptr<Unit>& unit_to_damage) const
{
	return damage;
}

int Miner::get_damage_frame() const
{
	return damage_frame;
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
	const int prev_gold_count_ = gold_count_in_bag_;
	packet >> gold_count_in_bag_;
	if(prev_gold_count_ > gold_count_in_bag_)
	{
		private_sound_manager.play_sound(money_sound);
		private_effect_manager.add_effect(std::make_unique<DropMoneyEffect>(sf::Vector2f{ x_, y_ }, prev_gold_count_));
	}
	gold_count_bar_.update();
	Unit::update_from_packet(packet);
}

void Swordsman::play_hit_sound() const
{
	if(current_frame_ == hit_frame)
		shared_sound_manager.play_sound(sward_hit);
}

void Swordsman::play_damage_sound() const
{
	shared_sound_manager.play_sound(sward_damage);
}

void Swordsman::play_kill_sound() const
{
	shared_sound_manager.play_sound(sward_kill);
}

int Miner::get_id() const
{
	return id;
}

Swordsman::Swordsman(const sf::Vector2f spawn_point, const texture_ID texture_id)
	: Unit(texture_id, spawn_point, max_health, sprite_params)
{
}

int Swordsman::get_id() const
{
	return id;
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

float Swordsman::get_damage(const std::shared_ptr<Unit>& unit_to_damage) const
{
	return damage;
}

int Swordsman::get_damage_frame() const
{
	return damage_frame;
}

int Swordsman::get_splash_count() const
{
	return splash_count;
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

void Magikill::play_hit_sound() const
{
	if (current_frame_ == hit_frame)
	{
		shared_sound_manager.play_sound(explosion_sound);
		shared_effects_manager.add_effect(std::make_unique<ExplosionEffect>(sf::Vector2f{x_, y_}, prev_direction_));
	}
	
}

Magikill::Magikill(const sf::Vector2f spawn_point, const texture_ID texture_id) :
	Unit(texture_id, spawn_point, max_health, sprite_params),
time_left_to_next_attack_bar_(attack_cooldown_time, time_left_to_next_attack_, spawn_point, Bar<int>::unit_bar_size, Bar<int>::unit_second_attribute_bar_offset, Bar<int>::magikill_cooldown_time_bar_color)
{

}

void Magikill::draw(DrawQueue& queue) const
{
	Unit::draw(queue);
	if (is_alive() and time_left_to_next_attack_ > 0)
		time_left_to_next_attack_bar_.draw(queue);
}

void Magikill::set_screen_place(const float camera_position)
{
	Unit::set_screen_place(camera_position);
	time_left_to_next_attack_bar_.set_position({ x_ - camera_position, y_ });
}

void Magikill::commit_attack()
{
	if(time_left_to_next_attack_ == 0)
		Unit::commit_attack();
}

bool Magikill::can_do_damage()
{
	if(time_left_to_next_attack_ == 0)
	{
		const bool temp = do_damage_flag_;
		do_damage_flag_ = false;
		if (temp)
			time_left_to_next_attack_ = attack_cooldown_time;
		return temp;
	}
	return false;
}

void Magikill::process(const sf::Time time)
{
	Unit::process(time);
	time_left_to_next_attack_ = std::max(time_left_to_next_attack_ - time.asMilliseconds(), 0);
	time_left_to_next_attack_bar_.update();
}

int Magikill::get_id() const
{
	return id;
}

int Magikill::get_places_requires() const
{
	return places_requires;
}

float Magikill::get_max_health() const
{
	return max_health;
}

sf::Vector2f Magikill::get_max_speed() const
{
	return max_speed;
}

float Magikill::get_damage(const std::shared_ptr<Unit>& unit_to_damage) const
{
	if (unit_to_damage == nullptr)
		return max_damage;
	return std::min(max_damage,  damage_factor * unit_to_damage->get_max_health());
}

int Magikill::get_damage_frame() const
{
	return damage_frame;
}

int Magikill::get_splash_count() const
{
	return splash_count;
}

int Magikill::get_stun_time() const
{
	return stun_time;
}

float Magikill::get_attack_distance() const
{
	return attack_distance;
}

int Magikill::get_wait_time() const
{
	return wait_time;
}

int Magikill::get_cost() const
{
	return cost;
}

void Magikill::write_to_packet(sf::Packet& packet) const
{
	packet << id << time_left_to_next_attack_;
	Unit::write_to_packet(packet);
}

void Magikill::update_from_packet(sf::Packet& packet)
{
	packet >> time_left_to_next_attack_;
	time_left_to_next_attack_bar_.update();
	Unit::update_from_packet(packet);
}

void Spearton::play_second_attack_hit_sound() const
{
	shared_sound_manager.play_sound(spearton_second_attack_sound);
}

void Spearton::push(const int direction)
{
	if(animation_type_ != second_attack_animation)
		Unit::push(direction);
}

bool Spearton::animation_complete()
{
	if (animation_type_ == second_attack_animation and second_attack_start_delay_time_ > 0)
		return true;
	return Unit::animation_complete();
}

void Spearton::play_damage_sound() const
{
	constexpr static int total_damage_sounds = 3;
	static std::uniform_int_distribution distribution(0, total_damage_sounds - 1);
	static std::mt19937 generator(std::random_device{}());

	const auto sound_id = static_cast<sound_buffer_id>(spearton_damage_0 + distribution(generator));

	shared_sound_manager.play_sound(sound_id);
}

void Spearton::play_kill_sound() const
{
	shared_sound_manager.play_sound(spearton_kill);
}

void Spearton::draw(DrawQueue& queue) const
{
	Unit::draw(queue);
	if (is_alive() and time_left_to_second_attack_ > 0)
		time_left_to_second_attack_bar_.draw(queue);
}

void Spearton::set_screen_place(float camera_position)
{
	Unit::set_screen_place(camera_position);
	time_left_to_second_attack_bar_.set_position({ x_ - camera_position, y_ });
}

Spearton::Spearton(sf::Vector2f spawn_point, texture_ID texture_id) : Unit(texture_id, spawn_point, max_health, sprite_params),
                                                                      time_left_to_second_attack_bar_(second_attack_cooldown_time, time_left_to_second_attack_, spawn_point, Bar<int>::unit_bar_size, Bar<int>::unit_second_attribute_bar_offset, Bar<int>::magikill_cooldown_time_bar_color)
{

}

void Spearton::process(const sf::Time time)
{
	Unit::process(time);

	time_left_to_second_attack_ = std::max(time_left_to_second_attack_ - time.asMilliseconds(), 0);
	time_left_to_second_attack_bar_.update();
	second_attack_start_delay_time_ = std::max(second_attack_start_delay_time_ - time.asMilliseconds(), 0);
}

void Spearton::commit_attack()
{
	Unit::commit_attack();
	current_damage_type_ = common;
}

bool Spearton::can_do_damage()
{
	if (current_damage_type_ == common)
		return Unit::can_do_damage();
	if (time_left_to_second_attack_ == 0)
	{
		const bool temp = do_damage_flag_;
		do_damage_flag_ = false;
		if (temp)
			time_left_to_second_attack_ = second_attack_cooldown_time;
		return temp;
	}
	return false;
}

void Spearton::commit_second_attack()
{
	if(time_left_to_second_attack_ == 0 and stun_time_left_ == 0)
	{
		shared_sound_manager.play_sound(spearton_second_attack_sound);
		current_damage_type_ = sparta;
		if (animation_type_ != second_attack_animation)
		{
			current_frame_ = 0;
		}
		set_animation_frame();
		animation_type_ = second_attack_animation;
		second_attack_start_delay_time_ = second_attack_start_delay_time;

		cumulative_time_++;
	}
}

int Spearton::get_id() const
{
	return id;
}

int Spearton::get_places_requires() const
{
	return places_requires;
}

float Spearton::get_max_health() const
{
	return max_health;
}

sf::Vector2f Spearton::get_max_speed() const
{
	return max_speed;
}

float Spearton::get_damage(const std::shared_ptr<Unit>&unit_to_damage) const
{
	if(current_damage_type_ == common)
		return damage;
	return second_attack_damage;
}

int Spearton::get_damage_frame() const
{
	return damage_frame;
}

int Spearton::get_splash_count() const
{
	if(current_damage_type_ == common)
		return Unit::get_splash_count();
	return second_attack_splash_count;
}

int Spearton::get_stun_time() const
{
	if(current_damage_type_ == common)
		return Unit::get_stun_time();
	return second_attack_stun_time;
}

int Spearton::get_second_attack_damage_frame() const
{
	return second_attack_damage_frame;
}

float Spearton::get_attack_distance() const
{
	if(current_damage_type_ == common)
		return attack_distance;
	return second_attack_distance;
}

int Spearton::get_wait_time() const
{
	return wait_time;
}

int Spearton::get_cost() const
{
	return cost;
}

void Spearton::write_to_packet(sf::Packet& packet) const
{
	packet << id << time_left_to_second_attack_;
	Unit::write_to_packet(packet);
}

void Spearton::update_from_packet(sf::Packet& packet)
{
	packet >> time_left_to_second_attack_;
	time_left_to_second_attack_bar_.update();
	Unit::update_from_packet(packet);
}

template <typename T>
void UnitFactory::register_unit(const int unit_id, texture_ID base_texture_id)
{
	factory_[unit_id] = [base_texture_id](const int player_num, const sf::Vector2f spawn_point)
		{
			return new T(spawn_point, Player::get_correct_texture_id(base_texture_id, player_num));
		};
}

Unit* UnitFactory::create_unit(const int id, const int player_num)
{
	return factory_[id](player_num, player_num >= 0 ? Player::spawn_point : enemy_spawn_point);
}

void UnitFactory::init()
{
	if(factory_.empty())
	{
		register_unit<Miner>(Miner::id, my_miner);
		register_unit<Swordsman>(Swordsman::id, my_swordsman);
		register_unit<Spearton>(Spearton::id, my_spearton);
		register_unit<Magikill>(Magikill::id, my_magikill);
	}
}



