#pragma once
#include "BaseState.h"
#include "MapObject.h"
#include "Units.h"
#include "UserInterface.h"

constexpr float max_camera_position = map_frame_width * 2;
constexpr float min_camera_position = 0;
constexpr float start_camera_position = 0;

static int enemy_behaviour = 0;

class ControlledUnit
{
	std::shared_ptr<Unit> unit_;
	sf::Sprite star_sprite_;

	inline const static sf::Vector2f star_scale = { 0.09f, 0.09f };
	inline const static sf::Vector2f star_shift = { -15, -10 };
	static constexpr float heal_factor = 0.2f;

public:
	static constexpr float speed_boost_factor = 1.5f;
	static constexpr float damage_boost_factor = 2.f;

	[[nodiscard]] std::shared_ptr<Unit> get_unit() const;
	void release();
	void draw(DrawQueue& queue);
	void heal() const;

	ControlledUnit(const std::shared_ptr<Unit>& unit);

	ControlledUnit& operator=(const std::shared_ptr<Unit>& new_unit);
};


class PlayState : public BaseState
{
	sf::Sprite background_sprite_;

	float camera_position_ = start_camera_position;

	std::unique_ptr<UserInterface> user_interface_;

	int money_ = 400;

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

	void move_camera(float step);
	void set_objects_screen_place() const;

public:
	PlayState();

	void update(sf::Time delta_time) override;
	void handle_input(Input& input, sf::Time delta_time) override;
	void draw(DrawQueue& draw_queue) override;
};

