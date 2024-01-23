#pragma once
#include "Army.h"
#include "Input.h"
#include "UserInterface.h"

class ControlledUnit
{
	std::shared_ptr<Unit> unit_;
	sf::Sprite star_sprite_;
	sf::Text name_text_;
	bool is_me_ = true;

	inline const static sf::Vector2f star_scale = { 0.08f, 0.08f };
	inline const static sf::Vector2f star_offset = { 0, -20 };

	inline const static sf::Vector2f name_scale = { 0.7f, 0.7f };
	inline const static sf::Vector2f name_offset = { 0, -60.f};

	static constexpr float heal_factor = 0.1f;

public:
	static constexpr float speed_boost_factor = 1.5f;
	static constexpr float damage_boost_factor = 3.f; //x3 x4

	std::optional<sf::Vector2f> last_position = {};
	[[nodiscard]] std::shared_ptr<Unit> get_unit() const;
	[[nodiscard]] bool get_is_me() const;
	int get_stun_time_left() const;

	void release();
	void draw(DrawQueue& queue);
	void heal() const;
	void set_y_scale();

	ControlledUnit(const std::shared_ptr<Unit>& unit, int id, const std::string& name);

	ControlledUnit& operator=(const std::shared_ptr<Unit>& new_unit);
};

class UserInterface;

class Player
{
	int player_id_;

	int money_ = 25000; //500
	float total_damage_ = 0;
	int total_kills_ = 0;

	std::unique_ptr<Army> army_;
	std::unique_ptr<SpawnUnitQueue> spawn_queue_;
	std::unique_ptr<ControlledUnit> controlled_unit_;
	std::unique_ptr<UserInterface> user_interface_;

	int timer_money_increment_ = 0;

	static constexpr int time_money_increment = 5000;
	static constexpr int count_money_increment = 20;

	void handle_change_controlled_unit(sf::Vector2i mouse_position) const;
public:
	inline static const sf::Vector2f spawn_point = { -100, 650 };
	static texture_ID get_correct_texture_id(texture_ID texture_id, int player_id);

	explicit Player(int player_id, const std::string& name = "");
	void set_screen_place(float camera_position) const;

	void update(sf::Time delta_time, Army& enemy_army, const std::shared_ptr<Statue>& enemy_statue, std::vector<std::shared_ptr<GoldMine>>& gold_mines);
	void draw(DrawQueue& draw_queue) const;
	void handle_input(const Input& input, int mouse_offset, sf::Time delta_time);

	[[nodiscard]] std::optional<sf::Vector2f> get_controlled_unit_position() const;

	Army& get_Army() const;
	[[nodiscard]] size_t get_id() const;

	void write_to_packet(sf::Packet& packet) const;
	void update_from_packet(sf::Packet& packet);
};

