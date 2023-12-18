#pragma once
#include <memory>
#include <queue>
#include <vector>
#include <optional>

#include "Units.h"

class Unit;
class Miner;

class Army
{
	constexpr static int max_soldiers_in_row = 5;
	constexpr static float row_width = 80;
	constexpr static int dead_unit_time_to_delete = 60000;

public:
	enum ArmyTarget
	{
		attack,
		escape,
		defend,
	};

	constexpr static int army_max_size = 40;

	constexpr static float defend_line_1 = 900;
	constexpr static float enemy_defend_line = map_frame_width * 3 - 600;

	Army(float army_defend_line, bool is_ally_army);

	void set_army_target(ArmyTarget target);
	[[nodiscard]] ArmyTarget get_army_target() const;

	const std::vector<std::shared_ptr<Unit>>& get_units() const;
	void add_unit(const std::shared_ptr<Unit>& unit);

	[[nodiscard]] int get_alive_units_count() const;

	void draw(DrawQueue& queue) const;
	void set_screen_place(float camera_position) const;

	int process(const Army& enemy_army, const std::shared_ptr<Statue>& enemy_statue, const std::shared_ptr<Unit>& controlled_unit, std::vector<std::shared_ptr<GoldMine>>& gold_mines, sf::Time delta_time);

	[[nodiscard]] bool is_ally() const;

protected:
	int process_miner(Miner* miner, const std::shared_ptr<Unit>& controlled_unit, std::vector<std::shared_ptr<GoldMine>>& gold_mines, sf::Time delta_time) const;
	void process_warrior(const std::shared_ptr<Unit>& unit, const std::shared_ptr<Unit>& controlled_unit, const Army& enemy_army, const std::shared_ptr<Statue>& enemy_statue, sf::Time delta_time);

	bool is_ally_army_;

	ArmyTarget army_target_ = defend;
	
	std::map<int, sf::Vector2f> defend_places_;
	std::vector<std::shared_ptr<Unit>> units_;
	std::vector<std::pair<std::shared_ptr<Unit>, int>> dead_units_;

	int alive_units_count_ = 0;
};


class SpawnUnitQueue
{
	std::queue<std::pair<std::shared_ptr<Unit>, int>> units_queue_;
	Army& army_;
public:

	SpawnUnitQueue(Army& army);

	void put_unit(const std::shared_ptr<Unit>& unit, int spawn_time);

	void process(sf::Time delta_time);

	[[nodiscard]] int get_free_places() const;

	[[nodiscard]] int get_army_count() const;
	[[nodiscard]] std::optional<int> get_front_unit_id() const;

	friend void process_enemy_spawn_queue(SpawnUnitQueue&, const Statue&);
};

bool random(float probability);

void process_enemy_spawn_queue(SpawnUnitQueue& queue, const Statue& enemy_statue);