#pragma once
#include <SFML/Graphics.hpp>
#include "TextureHolder.h"
#include "MapObject.h"

constexpr float X_MAP_MIN = -150;
constexpr float X_MAP_MAX = 2100 * 3 + 150;
constexpr float Y_MAP_MIN = 530;
constexpr float Y_MAP_MAX = 700;

constexpr int MINER_WAIT_TIME = 8000;
constexpr int MINER_COST = 250;
constexpr int SWARDSMAN_WAIT_TIME = 5000;
constexpr int SWARDSMAN_COST = 250;


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
		none,
		walk,
		die,
		attack
	};
	
protected:
	float _health;
	const float _max_health;
	float _armor;
	float _speed;
	float _damage;
	float _damage_speed;
	float _attack_distance;
	const float _vertical_speed = 0.25;
	int _spawn_time;

	
	AnimatonType _animation_type = none;

	sf::RectangleShape _health_bar;
	static int const _max_healthbar_size = 70;
	bool _killed = false;

	int _prev_direction = 1;
	bool _do_damage = false;

	Target _target = Target::defend;
	std::pair<int, sf::Vector2f> _stand_place = { 0,  { 1E+15f, 1E+15f } };

	void _update_health_bar();
	virtual void _set_y_scale() override;
public:

	constexpr static int ANIMATION_STEP = 70;
	std::shared_ptr<Unit> target_unit;

	Unit(TextureHolder& holder, ID id, sf::Vector2f spawnpoint, float _health, float init_y, float _speed, float _damage, float _damage_speed, float _attack_distance, AnimationParams _animation_params);

	Target get_target() const;
	float get_speed() const;
	int get_direction() const;
	float get_attack_distance() const;
	float get_damage() const;
	int get_spawn_time() const;
	virtual int get_places_requres() const = 0;
	
	bool animation_complete();
	void show_animation();
	
	virtual void move_sprite(sf::Vector2i vc) override;
	virtual void set_screen_place(int camera_position) override;
	virtual void move(sf::Vector2i direction, sf::Time time);
	virtual void commit_attack();
	virtual bool can_do_damage();
	void couse_damage(float _damage);
	bool is_alive() const;
	virtual void kill();
	bool is_killed();

	virtual void draw(sf::RenderWindow& window) const override;
	
	std::pair<int ,sf::Vector2f> get_stand_place() const;
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
	virtual int get_places_requres() const override;

	static Unit* MyMiner(sf::Vector2f spawnpoint, TextureHolder& holder);

	static Unit* EnemyMiner(sf::Vector2f spawnpoint, TextureHolder& holder);
};


class Swordsman : public Unit
{
	
public:
	const static int places_requres = 1;

	Swordsman(sf::Vector2f spawnpoint, TextureHolder& holder, ID id);
	virtual int get_places_requres() const override;

	static Unit* MySwordsman(sf::Vector2f spawnpoint, TextureHolder& holder);

	static Unit* EnemySwordsman(sf::Vector2f spawnpoint, TextureHolder& holder);
};

