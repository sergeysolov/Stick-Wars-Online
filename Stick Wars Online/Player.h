#pragma once
#include "Army.h"
#include "Input.h"


class ControlledUnit
{
	std::shared_ptr<Unit> unit_;
	sf::Sprite star_sprite_;

	inline const static sf::Vector2f star_scale = { 0.09f, 0.09f };
	inline const static sf::Vector2f star_shift = { -15, -10 };
	static constexpr float heal_factor = 0.2f;

public:
	static constexpr float speed_boost_factor = 1.5f;
	static constexpr float damage_boost_factor = 5.f;

	[[nodiscard]] std::shared_ptr<Unit> get_unit() const;
	void release();
	void draw(DrawQueue& queue);
	void heal() const;

	ControlledUnit(const std::shared_ptr<Unit>& unit);

	ControlledUnit& operator=(const std::shared_ptr<Unit>& new_unit);
};


class Player
{
	const size_t player_id_;

	int money_ = 500;
	Army army_;
	std::unique_ptr<SpawnUnitQueue> spawn_queue_;
	std::unique_ptr<ControlledUnit> controlled_unit_;

	int timer_money_increment_ = 0;
	static constexpr int time_money_increment = 5000;
	static constexpr int count_money_increment = 20;
public:
	Player(size_t player_id);
	void set_screen_place(float camera_position) const;

	void handle_mouse_input(sf::Vector2i mouse_position) const;
	std::optional<sf::Vector2f> handle_keyboard_input(const Input& input, sf::Time delta_time) const;
	void update(sf::Time delta_time, const Army& enemy_army, const std::shared_ptr<Statue>& enemy_statue, std::vector<std::shared_ptr<GoldMine>>& gold_mines);
	void draw(DrawQueue& draw_queue) const;

	Army& get_Army();
	int& get_money_count();
	SpawnUnitQueue& get_SpawnQueue() const;
};

