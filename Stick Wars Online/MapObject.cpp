#include "MapObject.h"
#include "MapObject.h"

MapObject::MapObject(sf::Vector2f spawnpoint, TextureHolder& holder, ID id, AnimationParams animation_params)
	: x_(spawnpoint.x), y_(spawnpoint.y), animation_params_(animation_params)
{
	sprite_.setTexture(holder.get_texture(id));
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

void MapObject::move_sprite(const sf::Vector2f offset)
{
	sprite_.move(offset.x, offset.y);
}

void MapObject::set_screen_place(const float camera_position)
{
	sprite_.setPosition({ x_ - camera_position, y_ });
}


void MapObject::draw(sf::RenderWindow& window) const
{
	window.draw(sprite_);
}

const MapObject::AnimationParams& MapObject::get_animation_params() const
{
	return animation_params_;
}

int MapObject::get_cumulative_time() const
{
	return cumulative_time_;
}

void MapObject::set_y_scale()
{
	const float scale_factor = a * sprite_.getPosition().y + b;
	sprite_.setScale({ scale_factor * animation_params_.scale.x, scale_factor * animation_params_.scale.y });
}


GoldMine::GoldMine(sf::Vector2f position, TextureHolder& holder) : MapObject(position, holder, goldmine,
	AnimationParams({ 0, 0 }, 538, 960, 10, { 0.2f, 0.2f }))
{

}

int GoldMine::mine(int gold_count)
{
	if (gold_count > gold_capacity_)
		gold_count = gold_capacity_;
	gold_capacity_ -= gold_count;
	current_frame_ = animation_params_.total_frames - 1 - animation_params_.total_frames *  gold_capacity_ / max_gold_capacity;
	sprite_.setTextureRect({ animation_params_.init_position.x + current_frame_ * animation_params_.frame_width, animation_params_.init_position.y, animation_params_.frame_width, animation_params_.frame_height });
	return gold_count;
}

bool GoldMine::empty() const
{
	return gold_capacity_ == 0;
}

Statue::Statue(sf::Vector2f position, TextureHolder& holder, ID id, float max_health) :
 MapObject(position, holder, id, AnimationParams({0, 0}, 817, 261, 1, {1.f, 1.f})), max_health_(max_health), health_(max_health),
health_bar_(max_health_, health_, position, {100, 15}) 
{

}

void Statue::cause_damage(const float damage)
{
	health_ = std::max(health_ - damage, 0.f);
	health_bar_.update();
}

void Statue::draw(sf::RenderWindow& window) const
{
	MapObject::draw(window);
	health_bar_.draw(window);
}

void Statue::set_screen_place(const float camera_position)
{
	MapObject::set_screen_place(camera_position);
	health_bar_.set_position({x_-camera_position, y_});
}

