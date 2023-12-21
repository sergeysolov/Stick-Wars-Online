#pragma once
#include "Army.h"
#include "Input.h"
#include "UserInterface.h"


class ControlledUnit
{
	std::shared_ptr<Unit> unit_;
	sf::Sprite star_sprite_;
	sf::Text name_text_;
	bool is_mine_ = true;

	inline const static sf::Vector2f star_scale = { 0.09f, 0.09f };
	inline const static sf::Vector2f star_shift = { -15, -10 };

	inline const static sf::Vector2f name_scale = { 0.8f, 0.8f };
	inline const static sf::Vector2f name_shift = { -30.f, -50.f};

	static constexpr float heal_factor = 0.2f;

public:
	static constexpr float speed_boost_factor = 1.5f;
	static constexpr float damage_boost_factor = 20.f;

	std::optional<sf::Vector2f> last_position = {};
	[[nodiscard]] std::shared_ptr<Unit> get_unit() const;
	[[nodiscard]] bool get_is_mine() const;

	void release();
	void draw(DrawQueue& queue);
	void heal() const;

	ControlledUnit(const std::shared_ptr<Unit>& unit, int id, const std::string& name);

	ControlledUnit& operator=(const std::shared_ptr<Unit>& new_unit);
};

class UserInterface;

class Player
{
	const size_t player_id_;

	int money_ = 500;

	std::unique_ptr<Army> army_;
	std::unique_ptr<SpawnUnitQueue> spawn_queue_;
	std::unique_ptr<ControlledUnit> controlled_unit_;
	std::unique_ptr<UserInterface> user_interface_;

	int timer_money_increment_ = 0;

	static constexpr int time_money_increment = 5000;
	static constexpr int count_money_increment = 20;
	inline static const sf::Vector2f spawn_point = { -100, 650 };

	static texture_ID get_correct_texture_id(texture_ID texture_id, size_t player_id);
public:
	Player(size_t player_id, const std::string& name = "");
	void set_screen_place(float camera_position) const;

	void update(sf::Time delta_time, Army& enemy_army, const std::shared_ptr<Statue>& enemy_statue, std::vector<std::shared_ptr<GoldMine>>& gold_mines);
	void draw(DrawQueue& draw_queue) const;
	void handle_input(const Input& input, sf::Time delta_time);

	[[nodiscard]] std::optional<sf::Vector2f> get_controlled_unit_position() const;

	[[nodiscard]] Army& get_Army() const;
	[[nodiscard]] size_t get_id() const;
};

