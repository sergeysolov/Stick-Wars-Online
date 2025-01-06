#pragma once
#include <array>
#include <unordered_set>
#include "TextureHolder.h"
#include "Attributes.h"
#include <SFML/Graphics.hpp>
#include <SFML/Network/Packet.hpp>

#include "DrawQueue.h"
#include "EffectsManager.h"

constexpr float map_frame_width = 2100;

constexpr float x_map_min = -200;
constexpr float x_map_max = map_frame_width * 3 + 200;
constexpr float y_map_min = 530;
constexpr float y_map_max = 700;

constexpr float gravity = 550.0f;


class MapObject
{
public:

	MapObject(sf::Vector2f spawn_point, texture_ID id, const SpriteParams& sprite_params);
	virtual ~MapObject() = default;

	const sf::Sprite& get_sprite() const;
	sf::Vector2f get_coords() const;
	virtual void set_screen_place(float camera_position);
	virtual void draw(DrawQueue& queue) const;
	const SpriteParams& get_animation_params() const;
	int get_cumulative_time() const;

	virtual void write_to_packet(sf::Packet& packet) const;
	virtual void update_from_packet(sf::Packet& packet);

	
protected:
	float x_;
	float y_;
	sf::Sprite sprite_;
	SpriteParams sprite_params_;
	uint16_t current_frame_ = 0;
	int cumulative_time_ = 0;

	constexpr static float scale_y_param_a = 13.0f / 4000, scale_y_param_b = -15.0f / 16;
	virtual float set_y_scale();
};


class BarbedWire : public MapObject
{
	inline const static SpriteParams sprite_params = { {0, 0}, 512, 678, {0.3f, 0.25f}, {} };
public:
	BarbedWire(sf::Vector2f position);
};


class GoldMine : public MapObject
{
public:
	static constexpr float offset = 70.f;
	inline const static std::array<sf::Vector2f, 16> goldmine_positions =
	{ sf::Vector2f{320, 785 + offset},
	{350, 670 + offset},
	{700, 790 + offset},
	{800, 670 + offset},
	{1100, 670 + offset},
	{1400, 690 + offset},
	{1150, 760 + offset},
	{1380, 780 + offset},
	{1950, 700 + offset},
	{2200, 760 + offset},
	{2400, 750 + offset},
	{2600, 690 + offset},
	{2650, 780 + offset},
	{map_frame_width * 3 - 1050, 700 + offset},
	{map_frame_width * 3 - 550, 800 + offset},
	{map_frame_width * 3 - 350, 680 + offset}, };

	const inline static SpriteParams sprite_params = SpriteParams({ 0, 0 }, 538, 960, { 0.2f, 0.2f }, { {1, 10 }});

	GoldMine(sf::Vector2f position);

	int mine(int gold_count);
	bool empty() const;
	static constexpr int max_gold_capacity = 4000; //4000

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
	inline const static SpriteParams sprite_params = SpriteParams({ 0, 0 }, 817, 261, { 1.f, 1.f }, {});
	constexpr static float my_max_health = 50000.0f;
	constexpr static float enemy_max_health = 60000.0f; //12000

	inline const static sf::Vector2f my_statue_position = { 500, 450 };
	inline const static sf::Vector2f enemy_statue_position = { map_frame_width * 3 - 800, 450 };

	Statue(sf::Vector2f position, texture_ID id, float max_health);

	float cause_damage(float damage);
	void draw(DrawQueue& queue) const override;
	void set_screen_place(float camera_position) override;
	bool is_destroyed() const;
	float get_health() const;

	void write_to_packet(sf::Packet& packet) const override;
	void update_from_packet(sf::Packet& packet) override;
};

class Arrow : public MapObject {
	inline const static SpriteParams sprite_params = { { 0, 0 }, 215, 1772, { 0.05f, 0.05f }, {} };
	static constexpr float time_speed_coeff = 1.35;
public:
	static constexpr float statue_damage_penalty_factor = 1.f / 3.f;
	constexpr static int collided_arrow_time_to_delete = 20000;

	Arrow& operator=(Arrow&) = default;

	Arrow();

	Arrow(const texture_ID id,
		  const sf::Vector2f spawn_point,
	      const sf::Vector2f velocity_,
		  const float damage,
		  const float ground_level);

	bool is_collided() const;
	float get_damage() const;
	float get_initial_y() const;
	bool add_damaged_unit(void* unit);
	int get_damaged_units_number_() const;
	sf::Vector2f get_velocity() const;


	void process(const sf::Time time);
	void draw(DrawQueue& queue) const override;

	void write_to_packet(sf::Packet& packet) const override;
	void update_from_packet(sf::Packet& packet) override;

private:
	const float initial_y_;
	bool is_collided_ = false;
	const float ground_level_;
	const float damage_;
	sf::Vector2f velocity_ = { 0.f, 0.f };

	std::unordered_set<void*> damaged_units_;
};
