#pragma once
#include <functional>
#include <memory>

#include <SFML/Graphics.hpp>

#include "TextureHolder.h"
#include "MapObject.h"
#include "Attributes.h"
#include "SoundBufHolder.h"

class Unit : public MapObject
{	
public:
	enum AnimationType
	{
		no_animation = -1,
		walk_animation,
		attack_animation,
		die_animation,
		defend_animation,
		defend_attack_animation,
		second_attack_animation
	};

	enum DamageType
	{
		no_damage,
		is_damage,
		is_kill
	};
	
protected:

	float health_; 
	Bar<float> health_bar_;

	sf::Vector2f speed_ = { 0.f, 0.f };
	constexpr static float acceleration = 0.0015f; // 0.002

	sf::Sprite stun_stars_sprite_;
	inline static const sf::Vector2f stun_sprite_offset = { -30.f,10.f };
	inline static const sf::Vector2f stun_sprite_scale = { 0.1f, 0.1f };
	int stun_time_left_ = 0;

	AnimationType animation_type_ = no_animation;
	std::pair<bool, bool> was_move_ = { false, false };
	int prev_direction_ = 1;

	bool dead_ = false;
	bool do_damage_flag_ = false;

	std::pair<int, sf::Vector2f> stand_place_ = { 0,  { 1E+15f, 1E+15f } };

	void set_y_scale() override;
	virtual bool animation_complete();
	void set_animation_frame(bool is_play_hit_sound=true);
	virtual void kill();
	virtual void push(int direction);

	virtual void play_hit_sound() const;
	virtual void play_second_attack_hit_sound() const;
public:
	virtual void play_damage_sound() const;
	virtual void play_kill_sound() const;

	constexpr static float trigger_attack_radius = 500.f;
	std::shared_ptr<Unit> target_unit;
	bool try_escape = false;

	Unit(texture_ID id, sf::Vector2f spawn_point, float health, const SpriteParams& animation_params);

	virtual int get_id() const = 0;
	virtual int get_places_requires() const = 0;
	virtual float get_max_health() const = 0;
	virtual sf::Vector2f get_max_speed() const = 0;
	virtual float get_damage() const = 0;
	virtual int get_splash_count() const;
	virtual int get_stun_time() const;
	virtual float get_attack_distance() const = 0;
	virtual int get_wait_time() const = 0;
	virtual int get_cost() const = 0;

	virtual int get_damage_frame() const = 0;
	virtual int get_second_attack_damage_frame() const;

	sf::Vector2f get_speed() const;
	int get_direction() const;

	void show_animation(int delta_time);
	void set_screen_place(float camera_position) override;

	virtual void process(sf::Time time);
	virtual void move(sf::Vector2i direction, sf::Time time);

	virtual void commit_attack();
	virtual void commit_second_attack();
	virtual bool can_do_damage();
	bool can_be_damaged() const;
	std::pair<float, DamageType> cause_damage(float damage, int direction, int stun_time);
	bool is_alive() const;
	
	bool was_killed();

	void draw(DrawQueue& queue) const override;

	sf::FloatRect get_unit_rect() const;
	std::pair<int, sf::Vector2f> get_stand_place() const;
	std::pair<int, sf::Vector2f> extract_stand_place();
	void set_stand_place(std::map<int, sf::Vector2f>& places);

	void write_to_packet(sf::Packet& packet) const override;
	void update_from_packet(sf::Packet& packet) override;
};

class Miner final : public Unit
{
	int gold_count_in_bag_ = 0;
	Bar<int> gold_count_bar_;

public:
	texture_ID base_texture_id = my_miner;
	constexpr static int id = 0;
	constexpr static int places_requires = 1;
	constexpr static float max_health = 100;
	inline const static sf::Vector2f max_speed = { 0.2f, 0.2f };
	constexpr static float damage = 10.f;
	constexpr static int damage_frame = 15;
	constexpr static float attack_distance = 150.0f;
	constexpr static int wait_time = 6000;
	constexpr static int cost = 250;
	constexpr static int gold_bag_capacity = 200;
	inline const static SpriteParams sprite_params = { {-50 / 2, 100 / 2}, 1080 / 4, 1920 / 4, {-0.6f * 2, 0.6f * 2}, {{50, 20}, //walk
																																															{50, 21}, // attack
																																															{40, 21}} }; // death
	//inline const static SpriteParams sprite_params = { {-50, 100}, 1080 / 2, 1920 / 2, {-0.6f, 0.6f}, 20, 50 };

	std::shared_ptr<GoldMine> attached_goldmine = nullptr;

	Miner(sf::Vector2f spawn_point, texture_ID texture_id);

	void draw(DrawQueue& queue) const override;
	void set_screen_place(float camera_position) override;

	void fill_bag(int gold_count);
	bool is_bag_filled() const;
	int flush_bag();

	int get_id() const override;
	int get_places_requires() const override;
	float get_max_health() const override;
	sf::Vector2f get_max_speed() const override;
	float get_damage() const override;
	float get_attack_distance() const override;
	int get_wait_time() const override;
	int get_cost() const override;

	int get_damage_frame() const override;

	void write_to_packet(sf::Packet& packet) const override;
	void update_from_packet(sf::Packet& packet) override;
};


class Swordsman final : public Unit
{
	void play_hit_sound() const override;
public:
	void play_damage_sound() const override;
	void play_kill_sound() const override;

	constexpr static int id = 1;
	constexpr static int places_requires = 1;
	constexpr static float max_health = 400; // 300
	inline const static sf::Vector2f max_speed = { 0.3f, 0.2f };
	constexpr static float damage = 40.f;
	constexpr static int damage_frame = 13;
	constexpr static int hit_frame = 2;
	constexpr static int splash_count = 3;
	constexpr static float attack_distance = 150.0f;
	constexpr static int wait_time = 3500;
	constexpr static int cost = 150;

	inline const static SpriteParams sprite_params = { {-50 / 2, 100 / 2}, 1080 / 4, 1920 / 4, {-0.6f * 2, 0.6f * 2}, {{40, 20},
																																															{30, 21},
																																															{45, 21}} };
	//inline const static SpriteParams sprite_params = { {-50, 100}, 1080 / 2, 1920 / 2, {-0.6f, 0.6f}, 20, 30 };

	Swordsman(sf::Vector2f spawn_point, texture_ID texture_id);

	int get_id() const override;
	int get_places_requires() const override;
	float get_max_health() const override;
	sf::Vector2f get_max_speed() const override;
	float get_damage() const override;
	int get_damage_frame() const override;
	int get_splash_count() const override;
	float get_attack_distance() const override;
	int get_wait_time() const override;
	int get_cost() const override;

	void write_to_packet(sf::Packet& packet) const override;
};

class Magikill final : public Unit
{
	int time_left_to_next_attack_ = 0;
	Bar<int> time_left_to_next_attack_bar_;

	void play_hit_sound() const override;
public:
	constexpr static int id = 2;
	constexpr static int places_requires = 8;
	constexpr static float max_health = 100;
	inline const static sf::Vector2f max_speed = { 0.2f, 0.15f }; // { 0.2f, 0.15f }
	constexpr static float damage = 25.f; //20
	constexpr static int damage_frame = 14;
	constexpr static int hit_frame = 14;
	constexpr static int splash_count = 1000;
	constexpr static int stun_time = 3000;
	constexpr static float attack_distance = 500.0f;
	constexpr static int wait_time = 15000; // 15000
	constexpr static int cost = 1600; // 1500
	constexpr static int attack_cooldown_time = 7000; // 7000
	inline const static SpriteParams sprite_params = { {-50 / 2, 150 / 2}, 1080 / 4, 1920 / 4, {-0.6f * 2, 0.6f * 2}, {{35, 21},
																																															{50, 21},
																																															{60, 21}} };
	//inline const static SpriteParams sprite_params = { {-50, 150}, 1080 / 2, 1920 / 2, {-0.6f, 0.6f}, 21, 40 };

	Magikill(sf::Vector2f spawn_point, texture_ID texture_id);

	void draw(DrawQueue& queue) const override;
	void set_screen_place(float camera_position) override;

	void commit_attack() override;
	bool can_do_damage() override;
	void process(sf::Time time) override;

	int get_id() const override;
	int get_places_requires() const override;
	float get_max_health() const override;
	sf::Vector2f get_max_speed() const override;
	float get_damage() const override;
	int get_damage_frame() const override;
	int get_splash_count() const override;
	int get_stun_time() const override;
	float get_attack_distance() const override;
	int get_wait_time() const override;
	int get_cost() const override;

	void write_to_packet(sf::Packet& packet) const override;
	void update_from_packet(sf::Packet& packet) override;
};


class Spearton final : public Unit
{
	enum damage_type
	{
		common,
		sparta
	};

	void play_second_attack_hit_sound() const override;
	void push(int direction) override;
	bool animation_complete() override;
public:
	constexpr static int id = 3;
	constexpr static int places_requires = 3;
	constexpr static float max_health = 2000;
	inline const static sf::Vector2f max_speed = { 0.25f, 0.175f }; // { 0.2f, 0.15f }
	constexpr static float damage = 100.f;
	constexpr static int damage_frame = 14;
	constexpr static float attack_distance = 200.0f;
	constexpr static int hit_frame = 14;
	constexpr static float second_attack_damage = 70.f;
	constexpr static int second_attack_splash_count = 12;
	constexpr static int second_attack_stun_time = 3000;
	constexpr static int second_attack_damage_frame = 15;
	constexpr static int second_attack_cooldown_time = 60000;
	constexpr static int second_attack_start_delay_time = 1500;
	constexpr static float second_attack_distance = 400.f;
	constexpr static int wait_time = 7000;
	constexpr static int cost = 600;
	inline const static SpriteParams sprite_params = { {-50 / 2, 150 / 2}, 1080 / 4, 1920 / 4, {-0.6f * 2, 0.6f * 2},
		{{40, 21},
					 {30, 21},
					 {45, 21},
					 {40, 13},
					 {30, 13},
					 {45, 21}} };

	void draw(DrawQueue& queue) const override;
	void set_screen_place(float camera_position) override;

	Spearton(sf::Vector2f spawn_point, texture_ID texture_id);
	void process(sf::Time time) override;
	void commit_attack() override;
	bool can_do_damage() override;
	void commit_second_attack() override;

	int get_id() const override;
	int get_places_requires() const override;
	float get_max_health() const override;
	sf::Vector2f get_max_speed() const override;
	float get_damage() const override;																	
	int get_damage_frame() const override;
	int get_splash_count() const override;
	int get_stun_time() const override;
	int get_second_attack_damage_frame() const override;
	float get_attack_distance() const override;
	int get_wait_time() const override;
	int get_cost() const override;

	void write_to_packet(sf::Packet& packet) const override;
	void update_from_packet(sf::Packet& packet) override;
private:
	damage_type current_damage_type_ = common;
	int time_left_to_second_attack_ = second_attack_cooldown_time;
	Bar<int> time_left_to_second_attack_bar_;
	int second_attack_start_delay_time_ = second_attack_start_delay_time;
};


class UnitFactory
{
	inline static std::unordered_map<int, std::function<Unit*(int, sf::Vector2f)>> factory_;

	template <typename T>
	static void register_unit(int unit_id, texture_ID base_texture_id);
public:
	static Unit* create_unit(int id, int player_num);

	static void init();
};
