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
	float speed_;
	float damage_;
	float attack_distance_;
	const float vertical_speed_ = 0.25;
	
	AnimationType animation_type_ = no_animation;

	HealthBar health_bar_;

	bool dead_ = false;

	int prev_direction_ = 1;
	bool do_damage_flag_ = false;

	std::pair<int, sf::Vector2f> stand_place_ = { 0,  { 1E+15f, 1E+15f } };

	void set_y_scale() override;
public:
	constexpr static int animation_step = 70;
	constexpr static float trigger_attack_radius = 500.f;
	std::shared_ptr<Unit> target_unit;

	Unit(TextureHolder& holder, ID id, sf::Vector2f spawn_point, float health, float speed, float damage, float attack_distance, int spawn_time, AnimationParams animation_params);
	~Unit() override = default;

	virtual ID get_id() const = 0;

	float get_speed() const;
	int get_direction() const;
	float get_attack_distance() const;
	float get_damage() const;
	virtual int get_places_requires() const = 0;
	
	bool animation_complete();
	void show_animation(int delta_time);
	
	void move_sprite(sf::Vector2f offset) override;
	void set_screen_place(float camera_position) override;
	virtual void move(sf::Vector2i direction, sf::Time time);
	virtual void commit_attack();
	virtual bool can_do_damage();
	void cause_damage(float damage);
	bool is_alive() const;
	virtual void kill();
	bool was_killed();

	void draw(sf::RenderWindow& window) const override;
	
	std::pair<int, sf::Vector2f> get_stand_place() const;
	std::pair<int, sf::Vector2f> extract_stand_place();
	void set_stand_place(std::map<int, sf::Vector2f>& places);
};

class Miner : public Unit
{
public:
	static constexpr ID texture_id = my_miner;
	constexpr static int places_requires = 1;
	constexpr static float max_health = 100;
	constexpr static float speed = 0.2f;
	constexpr static float damage = 200.f;
	constexpr static float attack_distance = 150.0f;
	constexpr static int wait_time = 6000;
	constexpr static int cost = 250;
	inline const static AnimationParams animation_params = { {-300, 2}, 700, 1280, 13, {-0.4f, 0.4f } };

	std::shared_ptr<GoldMine> attached_goldmine = nullptr;

	Miner(sf::Vector2f spawn_point, TextureHolder& holder, ID id);
	~Miner() override = default;

	int get_places_requires() const override;
	ID get_id() const override;
};


class Swordsman : public Unit
{
public:
	static constexpr ID texture_id = my_swordsman;
	constexpr static int places_requires = 1;
	constexpr static float max_health = 300;
	constexpr static float speed = 0.3f;
	constexpr static float damage = 30.f;
	constexpr static float attack_distance = 150.0f;
	constexpr static int wait_time = 3500;
	constexpr static int cost = 150;
	inline const static AnimationParams animation_params = { {-300, 20}, 700, 1280, 13, {-0.4f, 0.4f} };

	Swordsman(sf::Vector2f spawn_point, TextureHolder& holder, ID id);
	~Swordsman() override = default;

	int get_places_requires() const override;
	ID get_id() const override;
};
