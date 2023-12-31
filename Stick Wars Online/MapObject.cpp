#include "MapObject.h"
#include "MapObject.h"

MapObject::MapObject(const sf::Vector2f spawn_point, texture_ID id, const AnimationParams& animation_params)
	: x_(spawn_point.x), y_(spawn_point.y), animation_params_(animation_params)
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

const AnimationParams& MapObject::get_animation_params() const
{
	return animation_params_;
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

void MapObject::set_y_scale()
{
	
	const float scale_factor = scale_y_param_a * sprite_.getPosition().y + scale_y_param_b;
	sprite_.setScale({ scale_factor * animation_params_.scale.x, scale_factor * animation_params_.scale.y });
}


GoldMine::GoldMine(const sf::Vector2f position) : MapObject(position, goldmine, animation_params)
{

}

int GoldMine::mine(int gold_count)
{
 	if (gold_count > gold_capacity_)
		gold_count = gold_capacity_;
	gold_capacity_ -= gold_count;
	current_frame_ = static_cast<uint16_t>((1.f - static_cast<float>(gold_capacity_) / static_cast<float>(max_gold_capacity)) * static_cast<float>(animation_params_.total_frames));
	sprite_.setTextureRect({ animation_params_.init_position.x + current_frame_ * animation_params_.frame_width, animation_params_.init_position.y, animation_params_.frame_width, animation_params_.frame_height });
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
 MapObject(position, id, animation_params), max_health_(max_health), health_(max_health),
health_bar_(max_health_, health_, position, Bar<float>::statue_health_bar_size, Bar<float>::statue_health_bar_offset, Bar<float>::health_bar_color) 
{
	if (id == enemy_statue)
		sprite_.scale({ -1.f, 1.f });
}

void Statue::cause_damage(const float damage)
{
	const float prev_health = health_;
	health_ = std::clamp(health_ - damage, 0.f, max_health_);

	if (const float actual_damage = prev_health - health_; abs(actual_damage) > 1e-5)
		effects_manager.add_effect(std::make_unique<DropDamageEffect>(sf::Vector2f{ x_, y_ }, actual_damage));

	health_bar_.update();
}

void Statue::draw(DrawQueue& queue) const
{
	MapObject::draw(queue);
	health_bar_.draw(queue);
}

void Statue::set_screen_place(const float camera_position)
{
	MapObject::set_screen_place(camera_position);
	health_bar_.set_position({x_-camera_position, y_});
}

bool Statue::is_destroyed() const
{
	return health_ <= 0;
}

float Statue::get_health() const
{
	return health_;
}

void Statue::write_to_packet(sf::Packet& packet) const
{
	//MapObject::write_to_packet(packet);
	packet << health_;
}

void Statue::update_from_packet(sf::Packet& packet)
{
	//MapObject::update_from_packet(packet);
	packet >> health_;
	cause_damage(0);
}

