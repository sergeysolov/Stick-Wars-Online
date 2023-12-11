#pragma once
#include "TextureHolder.h"
#include "Attributes.h"
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

	MapObject(sf::Vector2f spawnpoint, TextureHolder& holder, ID id, AnimationParams animation_params);
	virtual ~MapObject() = default;

	const sf::Sprite& get_sprite() const;
	sf::Vector2f get_coords() const;
	virtual void move_sprite(sf::Vector2f offset);
	virtual void set_screen_place(float camera_position);
	virtual void draw(sf::RenderWindow& window) const;
	const AnimationParams& get_animation_params() const;
	int get_cumulative_time() const;

protected:
	float x_;
	float y_;
	sf::Sprite sprite_;
	AnimationParams animation_params_;
	uint16_t current_frame_ = 0;
	int cumulative_time_ = 0;

	virtual void set_y_scale();
};



class GoldMine : public MapObject
{
public:
	GoldMine(sf::Vector2f position, TextureHolder& holder);

	int mine(int gold_count);
	bool empty() const;
	const int max_gold_capacity = 3000;
protected:
	int gold_capacity_ = max_gold_capacity;
};



class Statue : public MapObject
{
	const float max_health_;
	float health_;
	HealthBar health_bar_;
public:
	constexpr static float my_max_health = 10000.0f;
	constexpr static float enemy_max_health = 12000.0f;
	Statue(sf::Vector2f position, TextureHolder& holder, ID id, float max_health);
	void cause_damage(float damage);
	void draw(sf::RenderWindow& window) const override;
	void set_screen_place(float camera_position) override;
};
