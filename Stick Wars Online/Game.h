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

class Game
{
	sf::RenderWindow main_window_;
	TextureHolder texture_holder_;
	sf::Sprite background_sprite_;

	float camera_position_ = start_camera_position;

	std::unique_ptr<UserInterface> user_interface_ = nullptr;

	int money_ = 1000;

	int timer_money_increment_ = 0;
	const int time_money_increment_ = 10000;
	int count_money_increment_ = 10;

	std::unique_ptr<Statue> my_statue_;
	std::vector<Army> armies_;
	std::unique_ptr<SpawnUnitQueue> my_spawn_queue_;
	std::shared_ptr<Unit> controlled_unit_ = nullptr;

	std::unique_ptr<Statue> enemy_statue_;
	Army enemy_army_;
	std::unique_ptr<SpawnUnitQueue> enemy_spawn_queue_;

	std::vector<std::shared_ptr<GoldMine>> gold_mines_;

	bool is_pressed_a_ = false;
	bool is_pressed_d_ = false;
	bool is_pressed_w_ = false;
	bool is_pressed_s_ = false;
	bool is_pressed_k_ = false;
	bool is_pressed_left_arrow_ = false;
	bool is_pressed_right_arrow_ = false;
	bool is_pressed_space_ = false;
	bool is_pressed_shift_ = false;
	bool is_mouse_left_button_clicked_ = false;

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



