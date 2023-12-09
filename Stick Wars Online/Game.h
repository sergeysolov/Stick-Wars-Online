#pragma once
#include <SFML/Graphics.hpp>
#include "TextureHolder.h"
#include <vector>
#include <queue>
#include <map>
#include "Units.h"
#include <memory>
#include <array>

constexpr int max_camera_position = 2100 * 2;
constexpr int min_camera_position = 0;
constexpr int start_camera_position = 0;

const sf::Vector2f spawnpoint = { -100, 650 };
constexpr float defendline_x = 900;
constexpr int total_defend_places = 40;
constexpr int max_solders_in_row = 5;
constexpr float row_width = 80;

constexpr float enemy_defendline_x = 2100 * 3 - 600;
const sf::Vector2f enemy_spawnpoint = { 2100 * 3 + 100, 650 };
constexpr int invoke_enemy_time = 15000;

static int enemy_behaviour = 0;

const std::array<sf::Vector2f, 9> goldmine_positions = { sf::Vector2f{150, 750}, {250, 670}, {350, 690}, {2100 * 3 - 350, 670}, {2100 * 3 - 450, 670}, {2100 * 3 - 550, 800},
	{700, 670}, {1000, 670}, {1300, 690} };

bool random(float probability);

class Button
{
protected:
	sf::Sprite sprite_;
	bool pressed_ = false;
public:
	Button(sf::Vector2f position, sf::Vector2f scale, ID id, TextureHolder& holder);
	const sf::Sprite& get_sprite() const;
	virtual void draw(sf::RenderWindow& window) const;
	virtual void press();
	bool is_pressed();
};

class UnitBuyButton : public Button
{
	sf::RectangleShape timebar_;
	const sf::Vector2f bar_size_ = { 85, 5 };

	int remaining_time_ = 0;
	const int wait_time_;

	sf::Text count_text_;
	sf::Sprite gold_icon_;
	sf::Text cost_text_;
	
	int unit_cost_;

public:
	UnitBuyButton(int unit_cost, int wait_time, sf::Vector2f position, sf::Vector2f scale, ID id, TextureHolder& holder, sf::Font& font);
	void draw(sf::RenderWindow& window) const override;
	int get_unit_cost() const;
	virtual void press() override;
	void process_button(int elapsed_time);
};


class Game
{
	sf::RenderWindow main_window_;
	TextureHolder texture_holder_;
	sf::Sprite background_sprite_;
	sf::Sprite gold_sprite_;

	int camera_position_ = start_camera_position;
	sf::Text camera_position_text_;

	sf::Sprite stick_man_;
	int army_count_ = 0;
	sf::Text army_count_text_;
	
	std::unique_ptr<UnitBuyButton> miner_buy_button_ = nullptr;
	std::unique_ptr<UnitBuyButton> swordsman_buy_button_ = nullptr;
	std::unique_ptr<Button> in_attack_button_ = nullptr;
	std::unique_ptr<Button> defend_button_ = nullptr;

	Target current_target_ = Target::defend;

	sf::Font text_font_;
	sf::Text money_count_text_;
	int money_ = 0;

	int timer_money_increment_ = 0;
	const int time_money_increment_ = 10000;
	int count_money_increment_ = 10;

	std::queue<std::shared_ptr<Unit>> units_queue_;
	int cumulative_spawn_time_ = 0;
	std::map<int, sf::Vector2f> defend_places_;
	std::vector<std::vector<std::shared_ptr<Unit>>> armies_;
	std::shared_ptr<Unit> controlled_unit_ = nullptr;

	std::vector<std::shared_ptr<Unit>> enemy_army_;
	std::map<int, sf::Vector2f> enemy_defend_places_;
	int enemy_army_count_ = 0;
	Target current_enemy_target_ = Target::defend;
	const int max_enemy_army_size_ = total_defend_places;
	int cumulative_enemy_spawn_time_ = 0;

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
	void handle_inputs(sf::Time deltatime);
	void process_internal_actions(sf::Time deltatime);

	int move_camera(int step);

	void add_money(int count);

	void add_enemy_unit(const std::shared_ptr<Unit>& unit);
	void add_gold_mine(sf::Vector2f position, TextureHolder& holder);
	static std::pair<bool, float> check_can_mine(const Miner* miner, const GoldMine* goldmine);
	static sf::Vector2f calculate_distances_to_mine(const Miner* miner, const GoldMine* goldmine);
	static sf::Vector2i calculate_direction_to_unit(const std::shared_ptr<Unit>& unit, const std::shared_ptr<Unit>& target_unit);
	static sf::Vector2f calculate_dx_dy_between_units(const std::shared_ptr<Unit>& unit, const std::shared_ptr<Unit>& target_unit);
	void damage_processing(const std::shared_ptr<Unit>& unit, std::vector<std::shared_ptr<Unit>>& enemy_army) const;
	std::shared_ptr<Unit> find_nearest_enemy_unit(const std::shared_ptr<Unit>& unit, std::vector<std::shared_ptr<Unit>> army) const;
	int unit_can_attack(const std::shared_ptr<Unit>& unit, std::vector<std::shared_ptr<Unit>>& enemy_army) const;
	void process_unit(std::shared_ptr<Unit> unit, std::vector<std::shared_ptr<Unit>>& enemy_army, std::map<int, sf::Vector2f>& defend_places, sf::Time deltatime, bool unit_from_my_army);
	void set_army_target(const std::vector<std::shared_ptr<Unit>>& army, Target target);

public:
	Game(uint16_t width, uint16_t height, const char* title);
	void init();
	int run();
};

