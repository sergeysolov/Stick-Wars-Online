#pragma once
#include "TextureHolder.h"
#include <SFML/Graphics.hpp>

constexpr float a = 13.0f / 4000, b = -15.0f / 16;

class MapObject
{
public:
	struct AnimationParams
	{
		sf::Vector2i init_position;
		int frame_height;
		int frame_width;
		int total_frames;
		sf::Vector2f scale;

		AnimationParams() = default;

		AnimationParams(sf::Vector2i init_position, uint16_t frame_height, uint16_t frame_width, uint16_t total_frames, sf::Vector2f scale)
			: init_position(init_position), frame_height(frame_height), frame_width(frame_width), total_frames(total_frames), scale(scale)
		{}
	};

	MapObject(sf::Vector2f spawnpoint, TextureHolder& holder, ID id, AnimationParams _animation_params);

	const sf::Sprite& get_sprite() const;
	sf::Vector2f get_coords() const;
	virtual void move_sprite(sf::Vector2i vc);
	virtual void set_screen_place(int camera_position);
	void add_time(int deltatime);
	virtual void draw(sf::RenderWindow& window) const;
	const AnimationParams& get_animation_params() const;
	int get_cumulative_time() const;

protected:
	float _x;
	float _y;
	sf::Sprite _sprite;
	AnimationParams _animation_params;
	uint16_t _current_frame = 0;
	int _cumulative_time = 0;

	virtual void _set_y_scale();
};


class GoldMine : public MapObject
{

public:
	GoldMine(sf::Vector2f position, TextureHolder& holder);

	int mine(int gold_count);
	bool empty() const;
protected:
	const int _max_gold_capacity = 10000;
	int _gold_capacity = _max_gold_capacity;
	

};