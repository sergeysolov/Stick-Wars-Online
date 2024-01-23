#include "Army.h"
#include "Game.h"

#include <random>
#include <ranges>

#include "PlayState.h"

void Army::play_in_attack_music(const bool play)
{
	static std::unique_ptr<sf::Sound> in_attack_sound;
	if(in_attack_sound == nullptr)
	{
		in_attack_sound = std::make_unique<sf::Sound>();
		in_attack_sound->setBuffer(sound_buffers_holder.get_sound_buffer(in_attack_music));
	}
}

Army::Army(const float army_defend_line, const int id, const int size_factor) : texture_shift_(id)
{
	max_size_ *= size_factor;
	if(is_ally())
	{
		for (int i = 0; i < max_size_; i++)
			defend_places_.insert({ i, { army_defend_line - (i / 5) * row_width, y_map_max - 30 - (i % max_soldiers_in_row) * (y_map_max - y_map_min - 50) / max_soldiers_in_row } });
	}
	else
	{
		for (int i = 0; i < max_size_; i++)
			defend_places_.insert({ max_size_ - i, {army_defend_line - (i / 5) * row_width, y_map_max - 30 - (i % max_soldiers_in_row) * (y_map_max - y_map_min - 50) / max_soldiers_in_row } });
	}
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

int Army::get_max_size() const
{
	return max_size_;
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

Army::ArmyReturnType Army::process(const std::vector<Army*>& enemy_armies, const std::shared_ptr<Statue>& enemy_statue, const std::shared_ptr<Unit>& controlled_unit, std::vector<std::shared_ptr<GoldMine>>& gold_mines, const sf::Time delta_time)
{
	ArmyReturnType result;

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

		const bool is_controlled_unit = unit == controlled_unit;
		unit->process(delta_time * (is_controlled_unit ? ControlledUnit::speed_boost_factor : 1.f));

		//Give stand place with less number to unit if place become free
		if (not defend_places_.empty() and unit->get_stand_place().first > defend_places_.begin()->first)
		{
			auto stand_place = unit->extract_stand_place();
			unit->set_stand_place(defend_places_);
			defend_places_.insert(stand_place);
		}

		// Process unit's behaviour
		if (const auto miner = dynamic_cast<Miner*>(unit.get()); miner != nullptr)
			result.gold_count += process_miner(miner, is_controlled_unit, gold_mines, delta_time);
		else
		{
			auto [damage, kills] = process_warrior(unit, is_controlled_unit, enemy_armies, enemy_statue, delta_time);
			if (is_controlled_unit)
			{
				result.damage += damage;
				result.kills += kills;
			}
		}
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
	return result;
}

void Army::process_client_locally(const sf::Time delta_time, const std::shared_ptr<Unit>& controlled_unit)
{
	for (auto it = units_.begin(); it != units_.end(); ++it)
	{
		auto& unit = *it;
		unit->show_animation(delta_time.asMilliseconds());
		unit->process(delta_time * (unit == controlled_unit ? ControlledUnit::speed_boost_factor : 1.f));
	}
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
	const auto prev_target = army_target_;
	int army_target;
	packet >> texture_shift_ >> army_target >> alive_units_count_;
	army_target_ = static_cast<ArmyTarget>(army_target);

	if (not is_ally() and (prev_target != attack and army_target == attack))
		play_in_attack_music();
	
	auto update_units_from_packet = [&](std::vector<std::shared_ptr<Unit>>& units, sf::Packet& packet_to_get_update)
		{
			size_t units_count;
			packet_to_get_update >> units_count;

			for (int i = 0; i < std::min(units_count, units.size()); i++)
			{
				int unit_id; packet_to_get_update >> unit_id;
				if (unit_id != units[i]->get_id())
					units[i] = std::shared_ptr<Unit>(UnitFactory::create_unit(unit_id, texture_shift_));
				units[i]->update_from_packet(packet_to_get_update);
			}

			if (units_count < units.size())
				units.resize(units_count);
			else
			{
				while (units.size() < units_count)
				{
					int unit_id; packet_to_get_update >> unit_id;
					std::shared_ptr<Unit> unit(UnitFactory::create_unit(unit_id, texture_shift_));
					unit->update_from_packet(packet_to_get_update);
					units.push_back(unit);
				}
			}
		};
	update_units_from_packet(units_, packet);
	const auto prev_dead_units_count = dead_units_.size();
	update_units_from_packet(dead_units_, packet);
}

int Army::process_miner(Miner* miner, bool is_controlled_unit, std::vector<std::shared_ptr<GoldMine>>& gold_mines, sf::Time delta_time) const
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
	{
		const float gold_count = (is_controlled_unit ? ControlledUnit::damage_boost_factor : 1) * miner->get_damage(nullptr);
		miner->fill_bag(find_nearest_goldmine()->get()->mine(static_cast<int>(gold_count)));
		shared_sound_manager.play_sound(miner_hit);
	}
		

	int gold_count_mined = 0;
	if (is_ally())
	{
		if (miner->get_coords().x <= escape_line + 10)
		{
			gold_count_mined = miner->flush_bag();
		}
	}
	else if (miner->get_coords().x >= enemy_escape_line - 10)
		miner->flush_bag();

	if (not is_controlled_unit)
	{
		if (miner->is_bag_filled() or army_target_ == escape)
		{
			int x_direction = is_ally() ? -1 : 1;
			miner->try_escape = true;
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

std::pair<float, int> Army::process_warrior(const std::shared_ptr<Unit>& unit, const bool is_controlled_unit, const std::vector<Army*>& enemy_armies, const std::shared_ptr<Statue>& enemy_statue, const sf::Time delta_time)
{
	auto calculate_dx_dy_between_units = [&](const std::shared_ptr<Unit>& unit_target) -> sf::Vector2f
		{
			const float dx = unit_target->get_coords().x - unit->get_coords().x;
			const float dy = unit_target->get_coords().y - unit->get_coords().y;
			return { dx, 5 * dy };
		};

	using unit_iterator = decltype(enemy_armies[0]->get_units().end());
	std::priority_queue < std::pair<float, unit_iterator>, std::vector < std::pair<float, unit_iterator>>, std::greater<>> enemies_by_distance;

	for (const auto enemy_army : enemy_armies)
	{
		for (auto it = enemy_army->get_units().begin(); it != enemy_army->get_units().end(); ++it)
		{
			auto [dx, dy] = calculate_dx_dy_between_units(*it);

			const float distance = abs(dx) + abs(dy);
			enemies_by_distance.emplace(distance, it);
		}
	}

	auto calculate_params_to_attack = [&](const unit_iterator& unit_it) -> std::pair<int, sf::Vector2f>
		{
			auto distance_to_nearest_enemy = calculate_dx_dy_between_units(*unit_it);
			auto [dx, dy] = distance_to_nearest_enemy;

			int is_enemy_nearby_to_attack = 0;
			if (abs(dx) <= unit->get_attack_distance() and abs(dy) <= unit->get_attack_distance())
				is_enemy_nearby_to_attack = (dx > 0 ? 1 : -1) * unit->get_direction();

			return { is_enemy_nearby_to_attack, distance_to_nearest_enemy };
		};


	const sf::Vector2f distance_to_statue = { enemy_statue->get_coords().x - unit->get_coords().x, 3 * (enemy_statue->get_coords().y + 180 - unit->get_coords().y) };
	int can_attack_statue = 0;
	if (abs(distance_to_statue.x) <= unit->get_attack_distance() and abs(distance_to_statue.y) <= unit->get_attack_distance())
		can_attack_statue = (enemy_statue->get_coords().x - unit->get_coords().x > 0 ? 1 : -1) * unit->get_direction();

	std::optional<unit_iterator> nearest_enemy = {};
	if (not enemies_by_distance.empty())
		nearest_enemy = enemies_by_distance.top().second;


	float caused_damage_by_controlled_unit = 0.f;
	int kill_count_by_controlled_unit = 0;

	// process causing damage
	if (unit->can_do_damage())
	{
		int enemies_damaged_count = 0;

		const auto base_damage_multiplayer = is_controlled_unit ? ControlledUnit::damage_boost_factor : 1;

		while (enemies_damaged_count < unit->get_splash_count() and not enemies_by_distance.empty())
		{
			const auto [is_enemy_nearby_to_attack, distance_to_nearest_enemy] = calculate_params_to_attack(enemies_by_distance.top().second);

			if(is_enemy_nearby_to_attack == 0)
				break;
			if(is_enemy_nearby_to_attack == 1)
			{
				float damage_multiplier = 0;
				const auto [dx, dy] = distance_to_nearest_enemy;
				if (dx >= 0 and unit->get_direction() > 0)
				{
					if (unit->get_direction() + enemies_by_distance.top().second->get()->get_direction() == 0)
						damage_multiplier = 1;
					else if (unit->get_direction() + enemies_by_distance.top().second->get()->get_direction() == 2)
						damage_multiplier = 2;
				}
				else if (dx < 0 and unit->get_direction() < 0)
				{
					if (unit->get_direction() + enemies_by_distance.top().second->get()->get_direction() == 0)
						damage_multiplier = 1;
					else if (unit->get_direction() + enemies_by_distance.top().second->get()->get_direction() == -2)
						damage_multiplier = 2;
				}

				damage_multiplier *= base_damage_multiplayer;
				const auto [actual_damage, damage_type] = enemies_by_distance.top().second->get()->cause_damage(unit->get_damage(*enemies_by_distance.top().second) * damage_multiplier, unit->get_direction(), unit->get_stun_time());
				if (damage_type == Unit::is_damage)
					unit->play_damage_sound();
				else if (damage_type == Unit::is_kill)
				{
					unit->play_kill_sound();
					kill_count_by_controlled_unit += 1;
				}
				caused_damage_by_controlled_unit += actual_damage;
				enemies_damaged_count += damage_multiplier > 1e-4;
			}
			enemies_by_distance.pop();
		}
  		if(can_attack_statue == 1 and enemies_damaged_count < unit->get_splash_count())
			caused_damage_by_controlled_unit += enemy_statue->cause_damage(unit->get_damage(nullptr) * base_damage_multiplayer);
	}

	const std::pair res = { caused_damage_by_controlled_unit, kill_count_by_controlled_unit };

	if (is_controlled_unit)
		return res;

	std::optional<std::pair<int, sf::Vector2f>> params_to_attack;

	if (army_target_ == escape)
	{
		int x_direction = is_ally() ? -1 : 1;
		unit->try_escape = true;
		unit->move({ x_direction, 0 }, delta_time);
		unit->target_unit.reset();
		return res;
	}

	if(nearest_enemy)
	{
		params_to_attack = calculate_params_to_attack(*nearest_enemy);
		if (abs(params_to_attack->second.x) + 450 * (*nearest_enemy)->get()->get_speed().x * (params_to_attack->second.x > 0 ? 1 : -1) <= unit->get_attack_distance() and abs(params_to_attack->second.y) <= unit->get_attack_distance())
		{
			if (params_to_attack->first < 0)
				unit->move({ -unit->get_direction(), 0 }, sf::Time(sf::milliseconds(1)));
			unit->commit_attack();

			const float health_ratio = unit->get_health() / unit->get_max_health();
			if ((army_target_ == defend and health_ratio < 0.9f) or (army_target_ == attack and health_ratio < 0.1f))
				unit->stand_defend();

			return res;
		}
	}

	if (army_target_ == defend)
	{
		bool go_to_stand_place = true;
		if (params_to_attack and abs(params_to_attack->second.x) < Unit::trigger_attack_radius and abs(params_to_attack->second.y / 5) < Unit::trigger_attack_radius)
		{
			go_to_stand_place = false;
			if (unit->target_unit == nullptr) // y / 5
				unit->target_unit = **nearest_enemy;
			if(not (**nearest_enemy)->can_be_damaged())
			{
				unit->target_unit.reset();
				go_to_stand_place = true;
			}
		}
		if(go_to_stand_place)
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
			return res;
		}


		float dist_to_enemy = 1E+15f;
		if(params_to_attack)
			dist_to_enemy = abs(params_to_attack->second.x) + abs(params_to_attack->second.y);
		const float dist_to_statue = abs(distance_to_statue.x) + abs(distance_to_statue.y);

		if (dist_to_statue < dist_to_enemy)
		{
			const auto [dx, dy] = distance_to_statue;
			const int x_direction = dx > 0 ? 1 : -1;
			const int y_direction = dy > 0 ? 1 : -1;
			const sf::Vector2i direction = { abs(dx) > 3 ? x_direction : 0, abs(dy) > 3 and abs(dx) < 200 ? y_direction : 0 };
			unit->move(direction, delta_time);
			return res;
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
	return res;
}

SpawnUnitQueue::SpawnUnitQueue(Army& army) : army_(army)
{
}

void SpawnUnitQueue::put_unit(const std::shared_ptr<Unit>& unit, const int spawn_time)
{
	units_queue_.emplace_back(unit, spawn_time);
	queue_size_ += unit->get_places_requires();
}

bool SpawnUnitQueue::remove_unit(const int unit_id)
{
	const auto find_res = std::ranges::find_if(units_queue_ | std::views::keys, [unit_id](const std::shared_ptr<Unit>& unit)
	{
		return unit->get_id() == unit_id;
	});

	if(find_res.base() != units_queue_.end())
	{
		queue_size_ -= find_res.base()->first->get_places_requires();
		units_queue_.erase(find_res.base());
		return true;
	}
	return false;
}

void SpawnUnitQueue::process(const sf::Time delta_time)
{
	if (not units_queue_.empty())
	{
		units_queue_.front().second -= delta_time.asMilliseconds();
		if (units_queue_.front().second <= 0)
		{
			army_.add_unit(units_queue_.front().first);
			queue_size_ -= units_queue_.front().first->get_places_requires();
			units_queue_.pop_front();
		}
	}
}

int SpawnUnitQueue::get_free_places() const
{
	return army_.get_max_size() - get_army_count();
}

int SpawnUnitQueue::get_army_count() const
{
	return army_.get_alive_units_count() + queue_size_;
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
	static constexpr int invoke_enemy_time = 8000;

	if (queue.units_queue_.empty() and queue.get_free_places() >= Swordsman::places_requires)
	{
		queue.put_unit(std::make_shared<Swordsman>(enemy_spawn_point, enemy_swordsman), invoke_enemy_time);
	}
	const int in_attack_threshold = queue.army_.get_max_size() / 8 * 7;
	if (queue.get_army_count() > in_attack_threshold and queue.army_.get_army_target() != Army::attack)
	{
		queue.army_.set_army_target(Army::attack);
		Army::play_in_attack_music();
	}
	else if (queue.get_army_count() < queue.army_.get_max_size() / 2)
	{
		queue.army_.set_army_target(Army::defend);
	}
	if(random(0.00004f) and queue.get_free_places() >= Miner::places_requires)
	{
		queue.army_.add_unit(std::make_shared<Miner>(enemy_spawn_point, enemy_miner));
	}
	const int spawn_threshold = queue.army_.get_max_size() / 8 * 7;
	if (queue.get_army_count() < spawn_threshold and random(0.0007f))
	{
		if (queue.get_free_places() >= Spearton::places_requires)
			queue.put_unit(std::shared_ptr<Unit>(UnitFactory::create_unit(Spearton::id, -1)), 1000);
		constexpr int count = 3;
		for (int i = 0; i < count and queue.get_free_places() >= Swordsman::places_requires; ++i)
			queue.put_unit(std::make_shared<Swordsman>(enemy_spawn_point, enemy_swordsman), 500);
	}

	static constexpr float reinforcement_count = 25;

	if(enemy_statue.get_health() < Army::prev_hit_points_of_enemy_statue - Statue::enemy_max_health / reinforcement_count)
	{
		Army::prev_hit_points_of_enemy_statue -= Statue::enemy_max_health / reinforcement_count;

		const int magikill_count = 2 * queue.army_.get_max_size() / Army::size_per_one_player;// / Army::size_per_one_player;
		const int count = queue.army_.get_max_size() / 4 * 3; // / 8

		for (int i = 0; i < magikill_count and queue.get_free_places() >= Magikill::places_requires; i++)
			queue.put_unit(std::shared_ptr<Unit>(UnitFactory::create_unit(Magikill::id, -1)), 100);

		for (int i = 0; i < count and queue.get_free_places() >= Swordsman::places_requires; ++i)
			queue.put_unit(std::make_shared<Swordsman>(enemy_spawn_point, enemy_swordsman), 100);
	}
}
