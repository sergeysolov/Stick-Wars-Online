#pragma once
#include <vector>
#include <queue>
#include <map>
#include <memory>
#include <array>
#include <memory>
#include <random>

#include <SFML/Graphics.hpp>

#include "TextureHolder.h"
#include "Units.h"
#include "Army.h"
#include "UserInterface.h"

constexpr float max_camera_position = map_frame_width * 2;
constexpr float min_camera_position = 0;
constexpr float start_camera_position = 0;

static int enemy_behaviour = 0;

struct PressedKeys
{
	bool a = false;
	bool d = false;
	bool w = false;
	bool s = false;
	bool k = false;
	bool left_arrow = false;
	bool right_arrow = false;
	bool space = false;
	bool shift = false;
	bool mouse_left = false;
};

class ControlledUnit
{
	std::shared_ptr<Unit> unit_;
	sf::Sprite star_sprite_;

	inline const static sf::Vector2f star_scale = { 0.09f, 0.09f };
	inline const static sf::Vector2f star_shift = { -15, -10 };
	static constexpr float health_increment = 0.2f;
	
public:
	static constexpr float speed_boost_factor = 2.0f;
	static constexpr float damage_boost_factor = 100.f;

	[[nodiscard]] std::shared_ptr<Unit> get_unit() const;
	void release();
	void draw(sf::RenderWindow& window);
	void heal() const;

	ControlledUnit(TextureHolder& holder, const std::shared_ptr<Unit>& unit);

	ControlledUnit& operator=(const std::shared_ptr<Unit>& new_unit);
};

class Game
{
	sf::RenderWindow main_window_;
	TextureHolder texture_holder_;
	sf::Sprite background_sprite_;

	float camera_position_ = start_camera_position;

	std::unique_ptr<UserInterface> user_interface_;

	int money_ = 1000;

	int timer_money_increment_ = 0;
	static constexpr int time_money_increment = 10000;
	int count_money_increment_ = 10;

	std::shared_ptr<Statue> my_statue_;
	std::vector<Army> armies_;
	std::unique_ptr<SpawnUnitQueue> my_spawn_queue_;

	std::unique_ptr<ControlledUnit> controlled_unit_;

	std::shared_ptr<Statue> enemy_statue_;
	Army enemy_army_;
	std::unique_ptr<SpawnUnitQueue> enemy_spawn_queue_;

	std::vector<std::shared_ptr<GoldMine>> gold_mines_;

	PressedKeys pressed_keys_;
	sf::Vector2i mouse_position_;

	sf::Clock clock_;

	void draw();
	void process_events();
	void handle_inputs(sf::Time delta_time);
	void process_internal_actions(sf::Time delta_time);

	void move_camera(float step);
	void set_objects_screen_place() const;

public:
	Game(uint16_t width, uint16_t height, const char* title);
	int run();
};



