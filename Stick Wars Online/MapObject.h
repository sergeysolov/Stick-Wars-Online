#pragma once
#include <array>
#include "TextureHolder.h"
#include "Attributes.h"
#include <SFML/Graphics.hpp>
#include <SFML/Network/Packet.hpp>

#include "DrawQueue.h"

constexpr float map_frame_width = 2100;

constexpr float x_map_min = -150;
constexpr float x_map_max = map_frame_width * 3 + 150;
constexpr float y_map_min = 530;
constexpr float y_map_max = 700;

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

		AnimationParams(sf::Vector2i init_position, uint16_t frame_height, uint16_t frame_width, uint16_t total_frames, sf::Vector2f scale)
			: init_position(init_position), frame_height(frame_height), frame_width(frame_width), total_frames(total_frames), scale(scale)
		{}
	};

	MapObject(sf::Vector2f spawnpoint, texture_ID id, const AnimationParams& animation_params);
	virtual ~MapObject() = default;

	const sf::Sprite& get_sprite() const;
	sf::Vector2f get_coords() const;
	virtual void set_screen_place(float camera_position);
	virtual void draw(DrawQueue& queue) const;
	const AnimationParams& get_animation_params() const;
	int get_cumulative_time() const;

	virtual void write_to_packet(sf::Packet& packet) const;
	virtual void update_from_packet(sf::Packet& packet);

protected:
	float x_;
	float y_;
	sf::Sprite sprite_;
	AnimationParams animation_params_;
	uint16_t current_frame_ = 0;
	int cumulative_time_ = 0;

	constexpr static float scale_y_param_a = 13.0f / 4000, scale_y_param_b = -15.0f / 16;
	virtual void set_y_scale();
};



class GoldMine : public MapObject
{
public:
	inline const static std::array<sf::Vector2f, 9> goldmine_positions = 
		{ sf::Vector2f{250, 750},
		{350, 670},
		{700, 790},
		{800, 670},
		{1100, 670},
		{1400, 690},
		{map_frame_width * 3 - 750, 700},
		{map_frame_width * 3 - 550, 800},
		{map_frame_width * 3 - 350, 680},};

	const inline static AnimationParams animation_params = AnimationParams({ 0, 0 }, 538, 960, 10, { 0.2f, 0.2f });

	GoldMine(sf::Vector2f position);

	int mine(int gold_count);
	bool empty() const;
	const int max_gold_capacity = 3000;

	void write_to_packet(sf::Packet& packet) const override;
	void update_from_packet(sf::Packet& packet) override;
protected:
	int gold_capacity_ = max_gold_capacity;
};



class Statue : public MapObject
{
	const float max_health_;
	float health_;
	Bar<float> health_bar_;
public:
	inline const static AnimationParams animation_params = AnimationParams({ 0, 0 }, 817, 261, 1, { 1.f, 1.f });
	constexpr static float my_max_health = 50000.0f;
	constexpr static float enemy_max_health = 60000.0f; //12000

	inline const static sf::Vector2f my_statue_position = { 500, 450 };
	inline const static sf::Vector2f enemy_statue_position = { map_frame_width * 3 - 800, 450 };

	Statue(sf::Vector2f position, texture_ID id, float max_health);

	void cause_damage(float damage);
	void draw(DrawQueue& queue) const override;
	void set_screen_place(float camera_position) override;
	bool is_destroyed() const;
	float get_health() const;

	void write_to_packet(sf::Packet& packet) const override;
	void update_from_packet(sf::Packet& packet) override;
};
