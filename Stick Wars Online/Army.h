﻿#pragma once
#include <memory>
#include <queue>
#include <vector>
#include <optional>
#include <SFML/Network/Packet.hpp>

#include "Units.h"

class Unit;
class Miner;

class Army
{
	constexpr static int max_soldiers_in_row = 5;
	constexpr static float row_width = 80;
	constexpr static int dead_unit_time_to_delete = 30000;
	
public:
	enum ArmyTarget
	{
		attack,
		escape,
		defend,
	};

	struct ArmyReturnType
	{
		int gold_count = 0;
		float damage = 0;
		int kills = 0;
	};

	constexpr static float escape_line = 150;
	constexpr static float enemy_escape_line = map_full_width - 150;

	inline static float prev_hit_points_of_enemy_statue;
	constexpr static std::array<float, 3> defend_lines = { 900, 1500, 2300 };
	constexpr static float enemy_defend_line = map_full_width - 600;
	constexpr static int size_per_one_player = 50;

	static void play_in_attack_music(bool play=true);

	Army(float army_defend_line, int id, int size_factor = 1);

	void set_army_target(ArmyTarget target);
	[[nodiscard]] ArmyTarget get_army_target() const;

	const std::vector<std::shared_ptr<Unit>>& get_units() const;
	void add_unit(const std::shared_ptr<Unit>& unit);

	[[nodiscard]] int get_alive_units_count() const;
	[[nodiscard]] int get_max_size() const;

	void draw(DrawQueue& queue) const;
	void set_screen_place(float camera_position);

	ArmyReturnType process(const std::vector<Army*>& enemy_armies, const std::shared_ptr<Statue>& enemy_statue, const std::shared_ptr<Unit>& controlled_unit, std::vector<std::shared_ptr<GoldMine>>& gold_mines, sf::Time delta_time);
	void process_client_locally(sf::Time delta_time, const std::shared_ptr<Unit>& controlled_unit) const;

	[[nodiscard]] bool is_ally() const;

	void write_to_packet(sf::Packet& packet) const;
	void update_from_packet(sf::Packet& packet);
	

protected:
	int process_miner(
		Miner* miner,
		bool is_controlled_unit,
		std::vector<std::shared_ptr<GoldMine>>& gold_mines,
		sf::Time delta_time) const;

	std::pair<float, int> process_warrior(
		const std::shared_ptr<Unit>& unit,
		bool is_controlled_unit,
		const std::vector<Army*>& enemy_armies,
		const std::shared_ptr<Statue>& enemy_statue,
		sf::Time delta_time);

	std::pair<float, int> process_arrows(
		std::vector<std::shared_ptr<Arrow>>& arrows,
		bool is_controled_unit,
		const std::vector<Army*>& enemy_armies,
		const std::shared_ptr<Statue>& enemy_statue,
		sf::Time delta_time);

	int max_size_ = size_per_one_player;
	int texture_shift_; // is equal to player id
	ArmyTarget army_target_ = defend;
	int alive_units_count_ = 0;

	std::map<int, sf::Vector2f> defend_places_;
	std::vector<std::shared_ptr<Unit>> units_;

	std::vector<std::shared_ptr<Unit>> dead_units_;
	std::vector<int> dead_units_remains_times_;
};

bool check_unit_in_line_with_arrow(Unit& unit, Arrow& arrow);

class SpawnUnitQueue
{
	std::deque<std::pair<std::shared_ptr<Unit>, int>> units_queue_;
	int queue_size_ = 0;
	Army& army_;
public:

	SpawnUnitQueue(Army& army);

	void put_unit(const std::shared_ptr<Unit>& unit, int spawn_time);
	bool remove_unit(int unit_id);

	void process(sf::Time delta_time);

	[[nodiscard]] int get_free_places() const;

	[[nodiscard]] int get_army_count() const;
	[[nodiscard]] std::optional<int> get_front_unit_id() const;

	friend void process_enemy_spawn_queue(SpawnUnitQueue&, const Statue&);
};

bool random(float probability);

static const sf::Vector2f enemy_spawn_point = { map_full_width + 100, 650 };

void process_enemy_spawn_queue(SpawnUnitQueue& queue, const Statue& enemy_statue);