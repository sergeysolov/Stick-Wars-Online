#include "MapObject.h"
#include <numbers>
#include <iostream>

#include "SoundBufHolder.h"

MapObject::MapObject(
	const sf::Vector2f spawn_point,
	texture_ID id,
	const SpriteParams& animation_params)
	: x_(spawn_point.x)
	, y_(spawn_point.y)
	, sprite_params_(animation_params)
{
	sprite_.setTexture(texture_holder.get_texture(id));
	sprite_.setTextureRect(sf::IntRect(animation_params.init_position.x, animation_params.init_position.y, animation_params.frame_width, animation_params.frame_height));
	sprite_.setOrigin({ static_cast<float>(animation_params.frame_width) / 2, 0 });
	sprite_.setPosition({ x_, y_ });
	MapObject::set_y_scale();
}

const sf::Sprite& MapObject::get_sprite() const
{
	return sprite_;
}

sf::Vector2f MapObject::get_coords() const
{
	return { x_, y_ };
}

void MapObject::set_screen_place(const float camera_position)
{
	sprite_.setPosition({ x_ - camera_position, y_ });
}


void MapObject::draw(DrawQueue& queue) const
{
	queue.emplace(map_object, &sprite_);
}

const SpriteParams& MapObject::get_animation_params() const
{
	return sprite_params_;
}

int MapObject::get_cumulative_time() const
{
	return cumulative_time_;
}

void MapObject::write_to_packet(sf::Packet& packet) const
{
	packet << x_ << y_ << current_frame_ << cumulative_time_;
}

void MapObject::update_from_packet(sf::Packet& packet)
{
	packet >> x_ >> y_ >> current_frame_ >> cumulative_time_;
	sprite_.setPosition({ x_, y_ });
	set_y_scale();
}

float MapObject::set_y_scale()
{
	const float scale_factor = scale_y_param_a * sprite_.getPosition().y + scale_y_param_b;
	sprite_.setScale({ scale_factor * sprite_params_.scale.x, scale_factor * sprite_params_.scale.y });
	return scale_factor;
}

BarbedWire::BarbedWire(const sf::Vector2f position) : MapObject(position, barbed_wire, sprite_params)
{
	if (x_ < (x_map_max + x_map_min) / 2)
	{
		sprite_.scale({ -1, 1 });
		sprite_.rotate(-60.f);
	}
	else
		sprite_.rotate(60.f);
}


GoldMine::GoldMine(const sf::Vector2f position) : MapObject(position, goldmine, sprite_params)
{
	sprite_.setOrigin( static_cast<float>(sprite_.getTextureRect().width) / 2, static_cast<float>(sprite_.getTextureRect().height) / 2);
}

int GoldMine::mine(int gold_count)
{
 	if (gold_count > gold_capacity_)
		gold_count = gold_capacity_;
	gold_capacity_ -= gold_count;
	current_frame_ = static_cast<uint16_t>((1.f - static_cast<float>(gold_capacity_) / static_cast<float>(max_gold_capacity)) * static_cast<float>(sprite_params_.animations[0].total_frames));
	sprite_.setTextureRect({ sprite_params_.init_position.x + current_frame_ * sprite_params_.frame_width, sprite_params_.init_position.y, sprite_params_.frame_width, sprite_params_.frame_height });
	return gold_count;
}

bool GoldMine::empty() const
{
	return gold_capacity_ == 0;
}

void GoldMine::write_to_packet(sf::Packet& packet) const
{
	MapObject::write_to_packet(packet);
	packet << gold_capacity_;
}

void GoldMine::update_from_packet(sf::Packet& packet)
{
	MapObject::update_from_packet(packet);
	packet >> gold_capacity_;
	mine(0);
}

Statue::Statue(sf::Vector2f position, texture_ID id, float max_health) :
 MapObject(position, id, sprite_params), max_health_(max_health), health_(max_health),
health_bar_(max_health_, health_, position, Bar<float>::statue_health_bar_size, Bar<float>::statue_health_bar_offset, Bar<float>::health_bar_color) 
{
	if (id == enemy_statue)
		sprite_.scale({ -1.f, 1.f });
}

float Statue::cause_damage(const float damage)
{
	const float prev_health = health_;
	health_ = std::clamp(health_ - damage, 0.f, max_health_);

	const float actual_damage = prev_health - health_;
	if (abs(actual_damage) > 1e-5)
		shared_effects_manager.add_effect(std::make_unique<DropDamageEffect>(sf::Vector2f{ x_, y_ }, actual_damage));

	health_bar_.update();

	if(actual_damage > 1e-5)
		shared_sound_manager.play_sound(statue_damage_sound);
	return actual_damage;
}

void Statue::draw(DrawQueue& queue) const
{
	MapObject::draw(queue);
	health_bar_.draw(queue);
}

void Statue::set_screen_place(const float camera_position)
{
	MapObject::set_screen_place(camera_position);
	health_bar_.set_position({x_ - camera_position, y_});
}

bool Statue::is_destroyed() const
{
	return health_ <= 1e-5;
}

float Statue::get_health() const
{
	return health_;
}

void Statue::write_to_packet(sf::Packet& packet) const
{
	packet << health_;
}

void Statue::update_from_packet(sf::Packet& packet)
{
	packet >> health_;
	cause_damage(0);
}

Arrow::Arrow()
	: MapObject({0, 0}, texture_ID::arrow, sprite_params)
	, initial_y_(0.f)
	, damage_(0.f)
	, velocity_({ 0.f, 0.f})
	, ground_level_(100000.f)
{
	sprite_.setOrigin(
		static_cast<float>(sprite_.getTextureRect().width) / 2,
		static_cast<float>(sprite_.getTextureRect().height) / 2);
}

Arrow::Arrow(
	const texture_ID id,
	const sf::Vector2f spawn_point,
	const sf::Vector2f velocity,
	const float damage,
	const float ground_level)
	: MapObject(spawn_point, id, sprite_params)
	, initial_y_(y_)
	, damage_(damage)
	, velocity_(velocity)
	, ground_level_(ground_level)
{
	sprite_.setOrigin(
		static_cast<float>(sprite_.getTextureRect().width) / 2,
		static_cast<float>(sprite_.getTextureRect().height) / 2);
}

void Arrow::process(sf::Time time)
{
	time *= time_speed_coeff;
	if (not is_collided_) {
		x_ += velocity_.x * time.asSeconds();
		y_ += velocity_.y * time.asSeconds();

		velocity_.y += gravity * time.asSeconds();

		const float speed = std::sqrt(velocity_.x * velocity_.x + velocity_.y * velocity_.y);
		float orientation = std::asin(velocity_.y / speed);
		if (velocity_.x < 0) {
			orientation = -orientation - std::numbers::pi_v<float>;
		}
		sprite_.setRotation(orientation * 180 / std::numbers::pi_v<float>);

		if (y_ + sprite_.getGlobalBounds().getSize().y > ground_level_) {
			is_collided_ = true;
		}
	}
}

void Arrow::draw(DrawQueue& queue) const
{
	queue.emplace(arrows, &sprite_);
}

void Arrow::write_to_packet(sf::Packet& packet) const
{
	packet << x_ << y_ << velocity_.x << velocity_.y << sprite_.getScale().x << sprite_.getScale().y;
}

void Arrow::update_from_packet(sf::Packet& packet)
{
	sf::Vector2f srite_scale;
	packet >> x_ >> y_ >> velocity_.x >> velocity_.y >> srite_scale.x >> srite_scale.y;
	sprite_.setPosition({ x_, y_ });
	sprite_.setScale(srite_scale);
	process(sf::milliseconds(0));
}

bool Arrow::is_collided() const
{
	return is_collided_;
}


float Arrow::get_damage() const
{
	return damage_;
}

float Arrow::get_initial_y() const
{
	return initial_y_;
}

bool Arrow::add_damaged_unit(void* unit)
{
	if (damaged_units_.contains(unit)) {
		return false;
	}
	damaged_units_.insert(unit);
	return true;
}

int Arrow::get_damaged_units_number_() const
{
	return static_cast<int>(damaged_units_.size());
}

sf::Vector2f Arrow::get_velocity() const
{
	return velocity_;
}
