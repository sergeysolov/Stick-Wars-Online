﻿#include "Army.h"
#include "Game.h"

#include <random>
#include <ranges>

#include "PlayState.h"

Army::Army(const float army_defend_line, const int id) : texture_shift_(id)
{
	if(is_ally())
		for (int i = 0; i < army_max_size; i++)
			defend_places_.insert({ i, { army_defend_line - (i / 5) * row_width, y_map_max - 30 - (i % max_soldiers_in_row) * (y_map_max - y_map_min - 50) / max_soldiers_in_row } });
	else
		for (int i = 0; i < army_max_size; i++)
			defend_places_.insert({ army_max_size - i, {army_defend_line - (i / 5) * row_width, y_map_max - 30 - (i % max_soldiers_in_row) * (y_map_max - y_map_min - 50) / max_soldiers_in_row } });

}

void Army::set_army_target(const ArmyTarget target)
{
	army_target_ = target;
}

Army::ArmyTarget Army::get_army_target() const
{
	return army_target_;
}

const std::vector<std::shared_ptr<Unit>>& Army::get_units() const
{
	return units_;
}

void Army::add_unit(const std::shared_ptr<Unit>& unit)
{
	units_.push_back(unit);
	alive_units_count_ += unit->get_places_requires();
}

int Army::get_alive_units_count() const
{
	return alive_units_count_;
}

void Army::draw(DrawQueue& queue) const
{
	for (const auto& unit : dead_units_)
		unit->draw(queue);

	for (const auto& unit : units_)
		unit->draw(queue);
}

void Army::set_screen_place(const float camera_position) const
{
	for (const auto& unit : units_)
		unit->set_screen_place(camera_position);

	for (const auto& dead_unit : dead_units_)
		dead_unit->set_screen_place(camera_position);
}

int Army::process(const std::vector<Army*>& enemy_armies, const std::shared_ptr<Statue>& enemy_statue, const std::shared_ptr<Unit>& controlled_unit, std::vector<std::shared_ptr<GoldMine>>& gold_mines, const sf::Time delta_time)
{
	int gold_count_mined = 0;
	for (auto it = units_.begin(); it != units_.end();)
	{
		auto& unit = *it;
		unit->show_animation(delta_time.asMilliseconds());

		// process unit if it was killed
		if (unit->was_killed())
		{
			auto stand_place = unit->extract_stand_place();
			if (stand_place.first >= 0)
				defend_places_.insert(stand_place);
			alive_units_count_ -= unit->get_places_requires();

			if(const auto miner = dynamic_cast<Miner*>(unit.get()); miner != nullptr)
				miner->attached_goldmine.reset();


			dead_units_.emplace_back(*it);
			dead_units_remains_times_.push_back(dead_unit_time_to_delete);
			it = units_.erase(it);
			continue;
		}

		unit->process_move(delta_time * (unit == controlled_unit ? ControlledUnit::speed_boost_factor : 1.f));

		//Give stand place with less number to unit if place become free
		if (not defend_places_.empty() and unit->get_stand_place().first > defend_places_.begin()->first)
		{
			auto stand_place = unit->extract_stand_place();
			unit->set_stand_place(defend_places_);
			defend_places_.insert(stand_place);
		}

		// Process unit's behaviour
		if (const auto miner = dynamic_cast<Miner*>(unit.get()); miner != nullptr)
			gold_count_mined += process_miner(miner, controlled_unit, gold_mines, delta_time);
		else
			process_warrior(unit, controlled_unit, enemy_armies, enemy_statue, delta_time);
		++it;
	}

	// process dead units
	for (int i = 0; i < dead_units_.size(); i++)
	{
		dead_units_[i]->show_animation(delta_time.asMilliseconds());
		dead_units_remains_times_[i] -= delta_time.asMilliseconds();
		if(dead_units_remains_times_[i] <= 0)
		{
			std::swap(dead_units_[i], dead_units_.back());
			dead_units_.pop_back();
			std::swap(dead_units_remains_times_[i], dead_units_remains_times_.back());
			dead_units_remains_times_.pop_back();
			break;
		}
	}
	return gold_count_mined;
}

bool Army::is_ally() const
{
	return texture_shift_ >= 0;
}

void Army::write_to_packet(sf::Packet& packet) const
{
	packet << texture_shift_ << army_target_ << alive_units_count_ << units_.size();
	for (const auto& unit : units_)
		unit->write_to_packet(packet);

	packet << dead_units_.size();
	for (const auto& dead_unit : dead_units_)
		dead_unit->write_to_packet(packet);
}

void Army::update_from_packet(sf::Packet& packet)
{
	int army_target;
	packet >> texture_shift_ >> army_target >> alive_units_count_;
	army_target_ = static_cast<ArmyTarget>(army_target);

	auto update_units_from_packet = [&](std::vector<std::shared_ptr<Unit>>& units, sf::Packet& packet_to_get_update)
		{
			size_t units_count;
			packet_to_get_update >> units_count;

			for (int i = 0; i < std::min(units_count, units.size()); i++)
			{
				int unit_id; packet_to_get_update >> unit_id;
				if (unit_id != units[i]->get_id())
				{
					if (unit_id == Miner::id)
						units[i] = std::make_shared<Miner>(Player::spawn_point, Player::get_correct_texture_id(my_miner, texture_shift_));
					else if (unit_id == Swordsman::id)
						units[i] = std::make_shared<Swordsman>(Player::spawn_point, Player::get_correct_texture_id(my_swordsman, texture_shift_));
				}
				units[i]->update_from_packet(packet_to_get_update);
			}

			if (units_count < units.size())
				units.resize(units_count);
			else
			{
				while (units.size() < units_count)
				{
					int unit_id; packet_to_get_update >> unit_id;
					std::shared_ptr<Unit> unit;
					if (unit_id == Miner::id)
						unit = std::make_shared<Miner>(Player::spawn_point, Player::get_correct_texture_id(my_miner, texture_shift_));
					else if (unit_id == Swordsman::id)
						unit = std::make_shared<Swordsman>(Player::spawn_point, Player::get_correct_texture_id(my_swordsman, texture_shift_));
					unit->update_from_packet(packet_to_get_update);
					units.push_back(unit);
				}
			}
		};
	update_units_from_packet(units_, packet);
	update_units_from_packet(dead_units_, packet);
}

int Army::process_miner(Miner* miner, const std::shared_ptr<Unit>& controlled_unit, std::vector<std::shared_ptr<GoldMine>>& gold_mines, sf::Time delta_time) const
{
	if (gold_mines.empty())
		return 0;

	auto check_can_mine = [&](const GoldMine* goldmine) -> std::pair<bool, sf::Vector2f>
		{
			const float dx = goldmine->get_coords().x - miner->get_coords().x;
			const float dy = 10 * (goldmine->get_coords().y - miner->get_coords().y - 120); // * 15,   -145

			bool can_mine = dx * static_cast<float>(miner->get_direction()) > 0 and abs((dy - 145)) <= miner->get_attack_distance() and abs(dx) <= miner->get_attack_distance();
			return { can_mine, {dx, dy} };
		};

	auto nearest_goldmine = gold_mines.end();
	auto find_nearest_goldmine = [&]
		{
			if (nearest_goldmine == gold_mines.end())
			{
				float nearest_distance = 1E+15f;
				for (auto it = gold_mines.begin(); it != gold_mines.end(); ++it)
				{
					const auto [_, dist_vector] = check_can_mine(it->get());
					const auto dist = abs(dist_vector.x) + abs(dist_vector.y);
					if (dist < nearest_distance)
					{
						nearest_distance = dist;
						nearest_goldmine = it;
					}
				}
			}
			return nearest_goldmine;
		};

	if (miner->can_do_damage() and check_can_mine(find_nearest_goldmine()->get()).first)
		miner->fill_bag(find_nearest_goldmine()->get()->mine(static_cast<int>(miner->get_damage())));

	int gold_count_mined = 0;
	if (is_ally())
	{
		if (miner->get_coords().x <= x_map_min + 200)
			gold_count_mined = miner->flush_bag();
	}
	else if (miner->get_coords().x >= x_map_max - 200)
		miner->flush_bag();

	if (miner != controlled_unit.get())
	{
		if (miner->is_bag_filled())
		{
			int x_direction = is_ally() ? -1 : 1;
			miner->move({ x_direction, 0 }, delta_time);
		}
		else if (miner->attached_goldmine != nullptr)
		{
			const auto [can_mine, distance_to_goldmine] = check_can_mine(miner->attached_goldmine.get());
			if (can_mine)
				miner->commit_attack();
			else
			{
				int x_direction = distance_to_goldmine.x > 0 ? 1 : -1;
				if (abs(distance_to_goldmine.x) > 180)
					miner->move({ x_direction, 0 }, delta_time);
				else
				{
					int y_direction = distance_to_goldmine.y > 0 ? 1 : -1;
					miner->move({ x_direction, y_direction }, delta_time);
				}
			}
			if (miner->attached_goldmine->empty())
				miner->attached_goldmine.reset();
		}
		else
		{
			if(is_ally())
			{
				for (const auto& goldmine : gold_mines)
					if (goldmine.use_count() == 1)
					{
						miner->attached_goldmine = goldmine;
						break;
					}
			}
			else
			{
				for (const auto& goldmine : gold_mines | std::views::reverse)
					if (goldmine.use_count() == 1)
					{
						miner->attached_goldmine = goldmine;
						break;
					}
			}
		}
	}
	if (find_nearest_goldmine()->get()->empty())
		gold_mines.erase(find_nearest_goldmine());

	return gold_count_mined;
}

void Army::process_warrior(const std::shared_ptr<Unit>& unit, const std::shared_ptr<Unit>& controlled_unit, const std::vector<Army*>& enemy_armies, const std::shared_ptr<Statue>& enemy_statue, sf::Time delta_time)
{
	auto calculate_dx_dy_between_units = [&](const std::shared_ptr<Unit>& unit_target) -> sf::Vector2f
		{
			const float dx = unit_target->get_coords().x - unit->get_coords().x;
			const float dy = unit_target->get_coords().y - unit->get_coords().y;
			return { dx, 5 * dy };
		};

	std::optional<decltype(enemy_armies[0]->get_units().end())> nearest_enemy = {};

	float nearest_distance = 1E+15f;
	for (const auto enemy_army : enemy_armies)
	{
		for (auto it = enemy_army->get_units().begin(); it != enemy_army->get_units().end(); ++it)
		{
			if (it->get()->is_alive())
			{
				auto [dx, dy] = calculate_dx_dy_between_units(*it);

				const float distance = abs(dx) + abs(dy);
				if (distance < nearest_distance)
				{
					nearest_distance = distance;
					nearest_enemy = it;
				}
			}
		}
	}

	int is_enemy_nearby_to_attack = 0; //no enemy
	sf::Vector2f distance_to_nearest_enemy = { 1E+15f, 1E+15f }; //inf, inf

	if (nearest_enemy)
	{
		distance_to_nearest_enemy = calculate_dx_dy_between_units(**nearest_enemy);
		auto [dx, dy] = distance_to_nearest_enemy;

		if (abs(dx) <= unit->get_attack_distance() and abs(dy) <= unit->get_attack_distance())
			is_enemy_nearby_to_attack = (dx > 0 ? 1 : -1) * unit->get_direction();
	}

	//const int direction_to_enemy_statue = (enemy_statue->get_coords().x - unit->get_coords().x > 0 ? 1 : -1) * unit->get_direction();
	const sf::Vector2f distance_to_statue = { enemy_statue->get_coords().x - unit->get_coords().x, 3 * (enemy_statue->get_coords().y + 180 - unit->get_coords().y) };
	int can_attack_statue = 0;
	if (abs(distance_to_statue.x) <= unit->get_attack_distance() and abs(distance_to_statue.y) <= unit->get_attack_distance())
		can_attack_statue = (enemy_statue->get_coords().x - unit->get_coords().x > 0 ? 1 : -1) * unit->get_direction();

	// process causing damage
	if (unit->can_do_damage())
	{
		if(is_enemy_nearby_to_attack == 1)
		{
			float damage_multiplier = 0;
			const auto [dx, dy] = distance_to_nearest_enemy;
			if (dx > 0 and unit->get_direction() == 1)
			{
				if (unit->get_direction() + (*nearest_enemy)->get()->get_direction() == 0)
					damage_multiplier = 1;
				else if (unit->get_direction() + (*nearest_enemy)->get()->get_direction() == 2)
					damage_multiplier = 2;
			}
			else if (dx < 0 and unit->get_direction() == -1)
			{
				if (unit->get_direction() + (*nearest_enemy)->get()->get_direction() == 0)
					damage_multiplier = 1;
				else if (unit->get_direction() + (*nearest_enemy)->get()->get_direction() == -2)
					damage_multiplier = 2;
			}

			if (unit == controlled_unit)
				damage_multiplier *= ControlledUnit::damage_boost_factor;

			(*nearest_enemy)->get()->cause_damage(unit->get_damage() * damage_multiplier, unit->get_direction());
		}
  		else if(can_attack_statue == 1)
  			enemy_statue->cause_damage(unit->get_damage() * (unit == controlled_unit ? ControlledUnit::damage_boost_factor : 1.f));
	}

	if (unit == controlled_unit)
		return;

	if(nearest_enemy)
		if (abs(distance_to_nearest_enemy.x) + 450 * (*nearest_enemy)->get()->get_speed().x * (distance_to_nearest_enemy.x > 0 ? 1 : -1) <= unit->get_attack_distance() and abs(distance_to_nearest_enemy.y) <= unit->get_attack_distance())
		{
			if (is_enemy_nearby_to_attack < 0)
				unit->move({ -unit->get_direction(), 0 }, sf::Time(sf::milliseconds(1)));
			unit->commit_attack();
			return;
		}

	if (army_target_ == defend)
	{
		if (abs(distance_to_nearest_enemy.x) < Unit::trigger_attack_radius and abs(distance_to_nearest_enemy.y / 5) < Unit::trigger_attack_radius) // y / 5
		{
			if (unit->target_unit == nullptr)
				unit->target_unit = **nearest_enemy;
		}
		else
		{
			unit->target_unit.reset();
			if (unit->get_stand_place().second.x > 1E+10)
				unit->set_stand_place(defend_places_);

			const float distance_x = unit->get_stand_place().second.x - unit->get_coords().x;
			const float distance_y = unit->get_stand_place().second.y - unit->get_coords().y;
			const int x_direction = abs(distance_x) > 10 ? (distance_x > 0 ? 1 : -1) : 0;
			const int y_direction = abs(distance_y) > 10 ? (distance_y > 0 ? 1 : -1) : 0;
			if (x_direction != 0 or y_direction != 0)
				unit->move({ x_direction, y_direction }, delta_time);
			else
			{
				if (is_ally())
				{
					if (unit->get_direction() < 0)
						unit->move({ 1, 0 }, sf::Time(sf::milliseconds(1)));
				}
				else
				{
					if (unit->get_direction() > 0)
						unit->move({ -1, 0 }, sf::Time(sf::milliseconds(1)));
				}
			}
		}
	}
	else if (army_target_ == attack)
	{
		if(unit->target_unit == nullptr and nearest_enemy)
			unit->target_unit = **nearest_enemy;

		if (can_attack_statue)
		{
			if (can_attack_statue < 0)
				unit->move({ -unit->get_direction(), 0 }, sf::Time(sf::milliseconds(1)));
			unit->commit_attack();
			return;
		}

		const float dist_to_enemy = abs(distance_to_nearest_enemy.x) + abs(distance_to_nearest_enemy.y);
		const float dist_to_statue = abs(distance_to_statue.x) + abs(distance_to_statue.y);

		if (dist_to_statue < dist_to_enemy)
		{
			const auto [dx, dy] = distance_to_statue;
			const int x_direction = dx > 0 ? 1 : -1;
			const int y_direction = dy > 0 ? 1 : -1;
			const sf::Vector2i direction = { abs(dx) > 3 ? x_direction : 0, abs(dy) > 3 and abs(dx) < 200 ? y_direction : 0 };
			unit->move(direction, delta_time);
			return;
		}
	}

	if (unit->target_unit != nullptr)
	{
		if (unit->target_unit->is_alive())
		{
			const auto [dx, dy] = calculate_dx_dy_between_units(unit->target_unit);
			const int x_direction = dx > 0 ? 1 : -1;
			const int y_direction = dy > 0 ? 1 : -1;
			const sf::Vector2i direction = { abs(dx) > 3 ? x_direction : 0, abs(dy) > 3 and abs(dx) < 200 ? y_direction : 0 };
			unit->move(direction, delta_time);
		}
		else
			unit->target_unit.reset();
	}
}

SpawnUnitQueue::SpawnUnitQueue(Army& army) : army_(army)
{
}

void SpawnUnitQueue::put_unit(const std::shared_ptr<Unit>& unit, const int spawn_time)
{
	units_queue_.emplace(unit, spawn_time);
}

void SpawnUnitQueue::process(const sf::Time delta_time)
{
	if (not units_queue_.empty())
	{
		units_queue_.front().second -= delta_time.asMilliseconds();
		if (units_queue_.front().second <= 0)
		{
			army_.add_unit(units_queue_.front().first);
			units_queue_.pop();
		}
	}
}

int SpawnUnitQueue::get_free_places() const
{
	return Army::army_max_size - get_army_count();
}

int SpawnUnitQueue::get_army_count() const
{
	return army_.get_alive_units_count() + units_queue_.size();
}

std::optional<int> SpawnUnitQueue::get_front_unit_id() const
{
	if (units_queue_.empty())
		return {};
	return units_queue_.front().first->get_id();
}

bool random(const float probability)
{
	if (probability <= 0)
		return false;
	if (probability >= 1)
		return true;

	static std::mt19937 generator{ std::random_device{}() };
	static std::uniform_real_distribution distribution(0.0f, 1.0f);

	return distribution(generator) < probability;
}

void process_enemy_spawn_queue(SpawnUnitQueue& queue, const Statue& enemy_statue)
{
	static const sf::Vector2f enemy_spawn_point = { map_frame_width * 3 + 100, 650 };
	static constexpr int invoke_enemy_time = 8000;

	if (queue.units_queue_.empty() and queue.get_free_places() >= Swordsman::places_requires)
	{
		queue.put_unit(std::make_shared<Swordsman>(enemy_spawn_point, enemy_swordsman), invoke_enemy_time);
	}
	if (queue.army_.get_units().size() > Army::army_max_size - 5)
	{
		queue.army_.set_army_target(Army::attack);
	}
	else if (queue.army_.get_units().size() < 15)
	{
		queue.army_.set_army_target(Army::defend);
	}
	if(random(0.00004f) and queue.get_free_places() >= Miner::places_requires)
	{
		queue.army_.add_unit(std::make_shared<Miner>(enemy_spawn_point, enemy_miner));
	}
	if (random(0.0008f))
	{
		int count = 3;
		if (enemy_statue.get_health() < Statue::enemy_max_health / 2)
			count += 3;
		if (enemy_statue.get_health() < Statue::enemy_max_health / 4)
			count += 3;
		for (int i = 0; i < count and queue.get_free_places() >= Swordsman::places_requires; ++i)
			queue.put_unit(std::make_shared<Swordsman>(enemy_spawn_point, enemy_swordsman), 500);
	}
}
