#pragma once
#include <vector>
#include <memory>

#include <SFML/Graphics.hpp>

#include "TextureHolder.h"
#include "MapObject.h"
#include "Attributes.h"

class Unit : public MapObject
{	
public:
	enum AnimationType
	{
		no_animation,
		walk_animation,
		die_animation,
		attack_animation
	};
	
protected:
	float health_;
	const float max_health_;
	sf::Vector2f speed_ = { 0.f, 0.f };
	const float max_speed_x_;
	inline constexpr static float max_speed_y = 0.20f;
	float damage_;
	float attack_distance_;
	
	AnimationType animation_type_ = no_animation;

	Bar<float> health_bar_;

	bool dead_ = false;

	std::pair<bool, bool> was_move_ = {false, false};
	int prev_direction_ = 1;
	constexpr static float acceleration = 0.003f;
	bool do_damage_flag_ = false;

	std::pair<int, sf::Vector2f> stand_place_ = { 0,  { 1E+15f, 1E+15f } };

	void set_y_scale() override;
	bool animation_complete();
	virtual void kill();
	void push(int direction);
public:
	constexpr static int animation_step = 70;
	constexpr static float trigger_attack_radius = 500.f;
	std::shared_ptr<Unit> target_unit;

	Unit(texture_ID id, sf::Vector2f spawn_point, float health, float speed, float damage, float attack_distance, int spawn_time, AnimationParams animation_params);
	~Unit() override = default;

	virtual texture_ID get_id() const = 0;

	sf::Vector2f get_speed() const;
	int get_direction() const;
	float get_attack_distance() const;
	float get_damage() const;
	virtual int get_places_requires() const = 0;
	float get_max_health() const;

	void show_animation(int delta_time);

	void set_screen_place(float camera_position) override;

	virtual void process_move(sf::Time time);
	virtual void move(sf::Vector2i direction, sf::Time time);

	virtual void commit_attack();
	virtual bool can_do_damage();
	void cause_damage(float damage, int direction);
	bool is_alive() const;
	
	bool was_killed();

	void draw(DrawQueue& queue) const override;
	
	std::pair<int, sf::Vector2f> get_stand_place() const;
	std::pair<int, sf::Vector2f> extract_stand_place();
	void set_stand_place(std::map<int, sf::Vector2f>& places);
};

class Miner : public Unit
{
	int gold_count_in_bag_ = 0;
	Bar<int> gold_count_bar_;
public:
	static constexpr texture_ID texture_id = my_miner;
	constexpr static int places_requires = 1;
	constexpr static float max_health = 100;
	constexpr static float speed = 0.2f;
	constexpr static float damage = 10.f;
	constexpr static float attack_distance = 150.0f;
	constexpr static int wait_time = 6000;
	constexpr static int cost = 250;
	constexpr static int gold_bag_capacity = 200;
	inline const static AnimationParams animation_params = { {-300, 2}, 700, 1280, 13, {-0.4f, 0.4f } };

	std::shared_ptr<GoldMine> attached_goldmine = nullptr;

	Miner(sf::Vector2f spawn_point, texture_ID id);
	~Miner() override = default;

	void draw(DrawQueue& queue) const override;
	void set_screen_place(float camera_position) override;

	void move(sf::Vector2i direction, sf::Time time) override;

	void fill_bag(int gold_count);
	bool is_bag_filled() const;
	int flush_bag();

	int get_places_requires() const override;
	texture_ID get_id() const override;
};


class Swordsman : public Unit
{
public:
	static constexpr texture_ID texture_id = my_swordsman;
	constexpr static int places_requires = 1;
	constexpr static float max_health = 300;
	constexpr static float speed = 0.3f;
	constexpr static float damage = 30.f;
	constexpr static float attack_distance = 150.0f;
	constexpr static int wait_time = 3500;
	constexpr static int cost = 150;
	inline const static AnimationParams animation_params = { {-300, 20}, 700, 1280, 13, {-0.4f, 0.4f} };

	Swordsman(sf::Vector2f spawn_point, texture_ID id);
	~Swordsman() override = default;

	int get_places_requires() const override;
	texture_ID get_id() const override;
};
