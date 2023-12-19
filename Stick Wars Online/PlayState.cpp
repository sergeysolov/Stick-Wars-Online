#include "PlayState.h"

void PlayState::set_objects_screen_place() const
{
	for (const auto& player : players_)
		player.set_screen_place(camera_position_);

	enemy_army_.set_screen_place(camera_position_);

	for (const auto& goldmine : gold_mines_)
		goldmine->set_screen_place(camera_position_);

	my_statue_->set_screen_place(camera_position_);
	enemy_statue_->set_screen_place(camera_position_);
}

void PlayState::move_camera(const float step)
{
	camera_position_ = std::clamp(camera_position_ + step, min_camera_position, max_camera_position);
	background_sprite_.setTextureRect({ static_cast<int>(camera_position_), 0, static_cast<int>(map_frame_width), 1050 });
}

PlayState::PlayState(StateManager& state_manager) : state_manager_(state_manager), enemy_army_(Army::enemy_defend_line, false)
{
	background_sprite_.setTexture(texture_holder.get_texture(large_forest_background));
	background_sprite_.setTextureRect({ static_cast<int>(start_camera_position), 0 ,static_cast<int>(map_frame_width), 1050 });

	players_.emplace_back(players_.size());

	user_interface_ = std::make_unique<UserInterface>(players_[0], state_manager);

	my_statue_ = std::make_shared<Statue>(Statue::my_statue_position, my_statue, Statue::my_max_health);
	enemy_statue_ = std::make_shared<Statue>(Statue::enemy_statue_position, enemy_statue, Statue::enemy_max_health);

	//Creating spawn queue
	enemy_spawn_queue_ = std::make_unique<SpawnUnitQueue>(enemy_army_);

	// Add goldmines
	for (const auto goldmine_position : GoldMine::goldmine_positions)
		gold_mines_.emplace_back(new GoldMine(goldmine_position));
}

void PlayState::update(const sf::Time delta_time)
{
	for (auto& player : players_)
	{
		enemy_army_.process(player.get_Army(), my_statue_, nullptr, gold_mines_, delta_time);
		player.update(delta_time, enemy_army_, enemy_statue_, gold_mines_);
	}

	user_interface_->update(delta_time, camera_position_);

	//Enemy army processing
	enemy_spawn_queue_->process(delta_time);

	if (enemy_behaviour == 0)
		process_enemy_spawn_queue(*enemy_spawn_queue_, *enemy_statue_);

	set_objects_screen_place();

	if (my_statue_->is_destroyed())
		state_manager_.switch_state(lose_menu);
	else if (enemy_statue_->is_destroyed())
		state_manager_.switch_state(victory_menu);
}

void PlayState::handle_input(Input& input, const sf::Time delta_time)
{
	// process behaviour of controlled unit
	//for (const auto& player : players_)//???????????????????????????????

	constexpr int player_id = 0;

	const auto controlled_unit_coords = players_[player_id].handle_keyboard_input(input, delta_time);
	if(controlled_unit_coords)
	{
		const float window_width = static_cast<float>(sf::VideoMode::getDesktopMode().width);
		const float shift = (controlled_unit_coords->x + 15 - window_width / 2 - camera_position_) / 15;
		move_camera(shift);
	}

	//process mouse clicks
	if (input.mouse_left)
		if (not user_interface_->process_left_mouse_button_press(input.mouse_position))
			players_[player_id].handle_mouse_input(input.mouse_position);


	if (input.left_arrow or input.right_arrow)
	{
		const int direction = -static_cast<int> (input.left_arrow) + static_cast<int> (input.right_arrow);
		const int shift = direction * delta_time.asMilliseconds() * 3;
		move_camera(static_cast<float>(shift));
	}
}

void PlayState::draw(DrawQueue& draw_queue)
{
	draw_queue.emplace(background, &background_sprite_);

	my_statue_->draw(draw_queue);
	enemy_statue_->draw(draw_queue);

	for (const auto& gold_mine : gold_mines_)
		gold_mine->draw(draw_queue);

	enemy_army_.draw(draw_queue);

	for (const auto& player : players_)
		player.draw(draw_queue);

	user_interface_->draw(draw_queue);
}

