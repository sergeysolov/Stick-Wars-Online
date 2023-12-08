#include "MapObject.h"

MapObject::MapObject(sf::Vector2f spawnpoint, TextureHolder& holder, ID id, AnimationParams animation_params)
	: x_(spawnpoint.x), y_(spawnpoint.y), animation_params_(animation_params)
{
	sprite_.setTexture(holder.getTexture(id));
	sprite_.setTextureRect(sf::IntRect(animation_params.init_position.x, animation_params.init_position.y, animation_params.frame_width, animation_params.frame_height));
	sprite_.setOrigin({ (float)animation_params.frame_width / 2, 0 });
	sprite_.setPosition({ x_, y_ });
	set_y_scale();
}

const sf::Sprite& MapObject::get_sprite() const
{
	return sprite_;
}

sf::Vector2f MapObject::get_coords() const
{
	return { x_, y_ };
}

void MapObject::move_sprite(sf::Vector2i vc)
{
	sprite_.move(static_cast<float>(vc.x), static_cast<float>(vc.y));
}

void MapObject::set_screen_place(int camera_position)
{
	sprite_.setPosition({ x_ - camera_position, y_ });
}

void MapObject::add_time(const int deltatime)
{
	cumulative_time_ += deltatime;
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


GoldMine::GoldMine(sf::Vector2f position, TextureHolder& holder) : MapObject(position, holder, ID::goldmine,
	AnimationParams({ 0, 0 }, 538, 960, 10, { 0.2f, 0.2f }))
{
	current_frame_ = animation_params_.total_frames - 1;
	sprite_.setTextureRect({ animation_params_.init_position.x + current_frame_ * animation_params_.frame_width, animation_params_.init_position.y, animation_params_.frame_width, animation_params_.frame_height });
}

int GoldMine::mine(int gold_count)
{
	if (gold_count > gold_capacity_)
		gold_count = gold_capacity_;
	gold_capacity_ -= gold_count;
	current_frame_ = animation_params_.total_frames * gold_capacity_ / max_gold_capacity_;
	sprite_.setTextureRect({ animation_params_.init_position.x + current_frame_ * animation_params_.frame_width, animation_params_.init_position.y, animation_params_.frame_width, animation_params_.frame_height });
	return gold_count;
}

bool GoldMine::empty() const
{
	return gold_capacity_ == 0;
}
