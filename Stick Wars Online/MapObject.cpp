#include "MapObject.h"

MapObject::MapObject(sf::Vector2f spawnpoint, TextureHolder& holder, ID id, AnimationParams _animation_params)
	: _x(spawnpoint.x), _y(spawnpoint.y), _animation_params(_animation_params)
{
	_sprite.setTexture(holder.getTexture(id));
	_sprite.setTextureRect(sf::IntRect(_animation_params.init_position.x, _animation_params.init_position.y, _animation_params.frame_width, _animation_params.frame_height));
	_sprite.setOrigin({ (float)_animation_params.frame_width / 2, 0 });
	_sprite.setPosition({ _x, _y });
	_set_y_scale();
}

const sf::Sprite& MapObject::get_sprite() const
{
	return _sprite;
}

sf::Vector2f MapObject::get_coords() const
{
	return { _x, _y };
}

void MapObject::move_sprite(sf::Vector2i vc)
{
	_sprite.move((float)vc.x, (float)vc.y);
}

void MapObject::set_screen_place(int camera_position)
{
	_sprite.setPosition({ _x - camera_position, _y });
}

void MapObject::add_time(int deltatime)
{
	_cumulative_time += deltatime;
}

void MapObject::draw(sf::RenderWindow& window) const
{
	window.draw(_sprite);
}

const MapObject::AnimationParams& MapObject::get_animation_params() const
{
	return _animation_params;
}

int MapObject::get_cumulative_time() const
{
	return _cumulative_time;
}

void MapObject::_set_y_scale()
{
	float scale_factor = (a * _sprite.getPosition().y + b);
	_sprite.setScale({ scale_factor * _animation_params.scale.x, scale_factor * _animation_params.scale.y });
}


GoldMine::GoldMine(sf::Vector2f position, TextureHolder& holder) : MapObject(position, holder, ID::goldmine,
	AnimationParams({ 0, 0 }, 538, 960, 10, { 0.2f, 0.2f }))
{
	_current_frame = _animation_params.total_frames - 1;
	_sprite.setTextureRect({ _animation_params.init_position.x + _current_frame * _animation_params.frame_width, _animation_params.init_position.y, _animation_params.frame_width, _animation_params.frame_height });
}

int GoldMine::mine(int gold_count)
{
	if (gold_count > _gold_capacity)
		gold_count = _gold_capacity;
	_gold_capacity -= gold_count;
	_current_frame = _animation_params.total_frames * _gold_capacity / _max_gold_capacity;
	_sprite.setTextureRect({ _animation_params.init_position.x + _current_frame * _animation_params.frame_width, _animation_params.init_position.y, _animation_params.frame_width, _animation_params.frame_height });
	return gold_count;
}

bool GoldMine::empty() const
{
	return _gold_capacity == 0;
}
