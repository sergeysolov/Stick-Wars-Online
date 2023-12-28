#pragma once
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
	constexpr static int dead_unit_time_to_delete = 60000;

	
public:
	enum ArmyTarget
	{
		attack,
		escape,
		defend,
	};

	inline static float prev_hit_points_of_enemy_statue;
	constexpr static std::array<float, 3> defend_lines = { 900, 1500, 2300 };
	constexpr static float enemy_defend_line = map_frame_width * 3 - 600;
	constexpr static int size_per_one_player = 40;

	static void play_in_attack_music(bool play=true);

	Army(float army_defend_line, int id, int size_factor = 1);

	void set_army_target(ArmyTarget target);
	[[nodiscard]] ArmyTarget get_army_target() const;

	const std::vector<std::shared_ptr<Unit>>& get_units() const;
	void add_unit(const std::shared_ptr<Unit>& unit);

	[[nodiscard]] int get_alive_units_count() const;
	[[nodiscard]] int get_max_size() const;

	void draw(DrawQueue& queue) const;
	void set_screen_place(float camera_position) const;

	int process(const std::vector<Army*>& enemy_armies, const std::shared_ptr<Statue>& enemy_statue, const std::shared_ptr<Unit>& controlled_unit, std::vector<std::shared_ptr<GoldMine>>& gold_mines, sf::Time delta_time);

	[[nodiscard]] bool is_ally() const;

	void write_to_packet(sf::Packet& packet) const;
	void update_from_packet(sf::Packet& packet);

protected:
	int process_miner(Miner* miner, const std::shared_ptr<Unit>& controlled_unit, std::vector<std::shared_ptr<GoldMine>>& gold_mines, sf::Time delta_time) const;
	void process_warrior(const std::shared_ptr<Unit>& unit, const std::shared_ptr<Unit>& controlled_unit, const std::vector<Army*>& enemy_armies, const std::shared_ptr<Statue>& enemy_statue, sf::Time delta_time);

	int max_size_ = size_per_one_player;
	int texture_shift_; // is equal to player id
	ArmyTarget army_target_ = defend;
	int alive_units_count_ = 0;

	std::map<int, sf::Vector2f> defend_places_;
	std::vector<std::shared_ptr<Unit>> units_;

	std::vector<std::shared_ptr<Unit>> dead_units_;
	std::vector<int> dead_units_remains_times_;
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