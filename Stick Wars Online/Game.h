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
const int invoke_enemy_time = 15000;

static int enemy_programm = 0;

const std::array<sf::Vector2f, 9> goldmine_positions = { sf::Vector2f{150, 750}, {250, 650}, {350, 690}, {2100 * 3 - 350, 670}, {2100 * 3 - 450, 670}, {2100 * 3 - 550, 800},
	{700, 650}, {1000, 650}, {1300, 690} };

bool random(float probability);

class Button
{
protected:
	sf::Sprite _sprite;
	bool _pressed = false;
public:
	Button(sf::Vector2f position, sf::Vector2f scale, ID id, TextureHolder& holder);
	const sf::Sprite& get_sprite() const;
	virtual void draw(sf::RenderWindow& window) const;
	virtual void press();
	bool is_pressed();
};

class UnitBuyButton : public Button
{
	sf::RectangleShape _timebar;
	const sf::Vector2f _bar_size = { 85, 5 };

	int _remaining_time = 0;
	const int _wait_time;

	sf::Text _count_text;
	sf::Sprite _gold_icon;
	sf::Text _cost_text;
	
	int _unit_cost;

public:
	UnitBuyButton(int unit_cost, int wait_time, sf::Vector2f position, sf::Vector2f scale, ID id, TextureHolder& holder, sf::Font& font);
	void draw(sf::RenderWindow& window) const override;
	int get_unit_cost() const;
	virtual void press() override;
	void process_button(int elapsed_time);
};


class Game
{
	sf::RenderWindow _main_window;
	TextureHolder _texture_holder;
	sf::Sprite _background_sprite;
	sf::Sprite _gold_sprite;

	int _camera_position = start_camera_position;
	sf::Text _camera_position_text;

	sf::Sprite _stick_man;
	int _army_count = 0;
	sf::Text _army_count_text;
	
	std::unique_ptr<UnitBuyButton> _miner_buy_button = nullptr;
	std::unique_ptr<UnitBuyButton> _swordsman_buy_button = nullptr;
	std::unique_ptr<Button> _in_attack_button = nullptr;
	std::unique_ptr<Button> _defend_button = nullptr;

	Target _current_target = Target::defend;

	sf::Font _textfont;
	sf::Text _money_count_text;
	int _money = 0;

	int _timer_money_increment = 0;
	const int _time_money_increment = 10000;
	int _count_money_increment = 10;

	std::queue<std::shared_ptr<Unit>> _units_queue;
	int _cumulative_spawn_time = 0;
	std::map<int, sf::Vector2f> _defend_places;
	std::vector<std::vector<std::shared_ptr<Unit>>> _armies;
	std::shared_ptr<Unit> _controlled_unit = 0;

	std::vector<std::shared_ptr<Unit>> _enemy_army;
	std::map<int, sf::Vector2f> _enemy_defend_places;
	int _enemy_army_count = 0;
	Target _current_enemy_target = Target::defend;
	const int _max_enemy_army_size = total_defend_places;
	int _cumulative_enemy_spawn_time = 0;

	std::vector<std::shared_ptr<GoldMine>> _gold_mines;

	bool _isPressed_A = false;
	bool _isPressed_D = false;
	bool _isPressed_W = false;
	bool _isPressed_S = false;
	bool _isPressed_K = false;
	bool _is_Pressed_Left_Arrow = false;
	bool _is_Pressed_Right_Arrow = false;
	bool _isPressed_Space = false;
	bool _isPressed_Shift = false;
	bool _isMouse_Left_Button_Clicked = false;
	sf::Vector2i _mouse_position;

	sf::Clock _clock;

	void _draw();
	void _process_events();
	void _handle_inputs(sf::Time deltatime);
	void _process_internal_actions(sf::Time deltatime);

	int _move_camera(int step);

	void _add_money(int count);
	void _add_unit(Unit* unit);
	void _add_unit(std::shared_ptr<Unit> unit);

	void _add_enemy_unit(std::shared_ptr<Unit> unit);
	void _add_gold_mine(sf::Vector2f position, TextureHolder& holder);
	static std::pair<bool, float> _check_can_mine(const Miner* miner, const GoldMine* goldmine);
	static sf::Vector2f _calculate_distances_to_mine(const Miner* miner, const GoldMine* goldmine);
	static sf::Vector2i _calculate_direction_to_unit(const Unit* unit, const Unit* target_unit);
	void _damage_processing(std::shared_ptr<Unit> unit, std::vector<std::shared_ptr<Unit>>& enemy_army);
	std::shared_ptr<Unit> _find_nearest_enemy_unit(std::shared_ptr<Unit> unit, std::vector<std::shared_ptr<Unit>> army);
	int _unit_can_attack(std::shared_ptr<Unit> unit, std::vector<std::shared_ptr<Unit>>& enemy_army) const;
	void _process_unit(std::shared_ptr<Unit> unit, std::vector<std::shared_ptr<Unit>>& enemy_army, std::map<int, sf::Vector2f>& defend_places, sf::Time deltatime, bool unit_from_my_army);
	void _set_army_target(std::vector<std::shared_ptr<Unit>>& army, Target target);

public:
	Game(uint16_t width, uint16_t height, const char* title);
	void init();
	int run();
};

