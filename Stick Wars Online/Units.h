#pragma once
#include <SFML/Graphics.hpp>
#include "TextureHolder.h"
#include "MapObject.h"

constexpr float x_map_min = -150;
constexpr float x_map_max = 2100 * 3 + 150;
constexpr float y_map_min = 530;
constexpr float y_map_max = 700;

constexpr int miner_wait_time = 6000;
constexpr int miner_cost = 250;
constexpr int swardsman_wait_time = 3500;
constexpr int swardsman_cost = 150;


//ÍÓ ×Å ÍÀÐÎÄ ÏÎÃÍÀËÈ ÍÀÕÓÉ (¨ÁÀÍÛÉ Â ÐÎÒ) -> ÂÑÅ
//íó íàÕÅÐ (ÂÎÇÂÐÀÒ)
//ñåìêè åñòü? à åñëè íàéäó?

enum Target
{
	attack,
	escape,
	defend,
};


class Unit : public MapObject
{	
public:
	enum AnimatonType
	{
		no_animation,
		walk_animation,
		die_animation,
		attack_animation
	};
	
protected:
	float health_;
	const float max_health_;
	float armor_;
	float speed_;
	float damage_;
	float damage_speed_;
	float attack_distance_;
	const float vertical_speed_ = 0.25;
	int spawn_time_;
	
	AnimatonType animation_type_ = no_animation;

	sf::RectangleShape health_bar_;
	static int constexpr max_healthbar_size = 70;
	bool killed_ = false;

	int prev_direction_ = 1;
	bool do_damage_ = false;

	Target target_ = Target::defend;
	std::pair<int, sf::Vector2f> stand_place_ = { 0,  { 1E+15f, 1E+15f } };

	void update_health_bar();
	void set_y_scale() override;
public:

	constexpr static int animation_step = 70;
	std::shared_ptr<Unit> target_unit;

	Unit(TextureHolder& holder, ID id, sf::Vector2f spawnpoint, float health, float init_y, float speed, float damage, float damage_speed, float attack_distance, AnimationParams animation_params);

	Target get_target() const;
	float get_speed() const;
	int get_direction() const;
	float get_attack_distance() const;
	float get_damage() const;
	int get_spawn_time() const;
	virtual int get_places_requres() const = 0;
	
	bool animation_complete();
	void show_animation();
	
	void move_sprite(sf::Vector2i vc) override;
	void set_screen_place(int camera_position) override;
	virtual void move(sf::Vector2i direction, sf::Time time);
	virtual void commit_attack();
	virtual bool can_do_damage();
	void couse_damage(float _damage);
	bool is_alive() const;
	virtual void kill();
	bool is_killed();

	void draw(sf::RenderWindow& window) const override;
	
	std::pair<int, sf::Vector2f> get_stand_place() const;
	std::pair<int, sf::Vector2f> extract_stand_place();
	void set_stand_place(std::map<int, sf::Vector2f>& places);
	void set_target(Target target);
	
};


class Miner : public Unit
{
	
public:
	const static int places_requres = 1;

	std::shared_ptr<GoldMine> attached_goldmine = nullptr;

	Miner(sf::Vector2f spawnpoint, TextureHolder& holder, ID id);
	int get_places_requres() const override;

	static Unit* MakeMiner(sf::Vector2f spawnpoint, TextureHolder& holder);

	static Unit* EnemyMiner(sf::Vector2f spawnpoint, TextureHolder& holder);
};


class Swordsman : public Unit
{
	//static constexpr ID texture_id = ID:: 
public:
	const static int places_requres = 1;

	Swordsman(sf::Vector2f spawnpoint, TextureHolder& holder, ID id);
	int get_places_requres() const override;

	static Unit* MakeSwordsman(sf::Vector2f spawnpoint, TextureHolder& holder);

	static Unit* EnemySwordsman(sf::Vector2f spawnpoint, TextureHolder& holder);
};

