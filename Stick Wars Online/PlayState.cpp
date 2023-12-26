#include "PlayState.h"

#include <ranges>

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

PlayState::PlayState(StateManager& state_manager) : state_manager_(state_manager), enemy_army_(Army::enemy_defend_line, -1)
{
	background_sprite_.setTexture(texture_holder.get_texture(large_forest_background));
	background_sprite_.setTextureRect({ static_cast<int>(start_camera_position), 0 ,static_cast<int>(map_frame_width), 1050 });

	camera_position_text_.setFont(text_font);
	camera_position_text_.setPosition(1800, 10);

	pause_button_ = std::make_unique<Button>(sf::Vector2f{ 1700.f, 20.f }, sf::Vector2f{ 0.15f, 0.15f }, pause_button);

	my_statue_ = std::make_shared<Statue>(Statue::my_statue_position, my_statue, Statue::my_max_health);
	enemy_statue_ = std::make_shared<Statue>(Statue::enemy_statue_position, enemy_statue, Statue::enemy_max_health);

	for (const auto goldmine_position : GoldMine::goldmine_positions)
		gold_mines_.emplace_back(new GoldMine(goldmine_position));

	if(client_handler == nullptr)
	{
		if (server_handler != nullptr)
		{
			//std::this_thread::sleep_for(std::chrono::milliseconds(200));
			players_.emplace_back(0, server_handler->get_player_name());
			for (const auto& client : server_handler->get_connections())
				players_.emplace_back(client.get_id(), client.get_name());

			sf::Packet players_info;
			players_info << static_cast<int>(server_handler->get_connections().size() + 1);
			players_info << 0 << server_handler->get_player_name();
			for (const auto& client : server_handler->get_connections())
				players_info << client.get_id() << client.get_name();

			for (const auto& client : server_handler->get_connections())
				client.get_socket().send(players_info);

			server_handler->start_receive_input();
			server_handler->start_send_updates();
		}
		else
			players_.emplace_back(0);
		enemy_spawn_queue_ = std::make_unique<SpawnUnitQueue>(enemy_army_);
	}
	else
	{
		sf::Packet players_info;
		client_handler->get_server().get_socket().receive(players_info);
		int players_number;
		players_info >> players_number;
		for (int i = 0; i < players_number; i++)
		{
			int id; std::string player_name;
			players_info >> id >> player_name;
			players_.emplace_back(id, player_name);
		}
		client_handler->start_send_input();
		client_handler->start_receive_updates();
	}
}

PlayState::~PlayState()
{
	if (client_handler != nullptr)
	{
		client_handler->stop_send_input();
		client_handler->stop_receive_updates();
	}
}

void PlayState::update(const sf::Time delta_time)
{
	if(client_handler == nullptr)
	{
		std::vector<Army*> ally_armies;
		for (auto& player : players_)
		{
			ally_armies.push_back(&player.get_Army());
			player.update(delta_time, enemy_army_, enemy_statue_, gold_mines_);
		}

		enemy_army_.process(ally_armies, my_statue_, nullptr, gold_mines_, delta_time);
		//if (enemy_behaviour == 0)
		process_enemy_spawn_queue(*enemy_spawn_queue_, *enemy_statue_);
		enemy_spawn_queue_->process(delta_time);

		if(server_handler != nullptr)
		{
			sf::Packet update_packet;
			update_packet << players_.size();
			for (auto& player : players_)
				player.write_to_packet(update_packet);

			my_statue_->write_to_packet(update_packet);
			enemy_statue_->write_to_packet(update_packet);

			enemy_army_.write_to_packet(update_packet);

			update_packet << gold_mines_.size();
			for (const auto& gold_mine : gold_mines_)
				gold_mine->write_to_packet(update_packet);

			server_handler->put_update_to_clients(update_packet);
		}
	}
	else
	{
		if(const auto packet = client_handler->get_update_from_server(); packet != nullptr)
		{
			size_t players_count;
			*packet >> players_count;
			for (auto& player : players_)
				player.update_from_packet(*packet);

			my_statue_->update_from_packet(*packet);
			enemy_statue_->update_from_packet(*packet);

			enemy_army_.update_from_packet(*packet);

			size_t gold_mines_count;
			*packet >> gold_mines_count;
			gold_mines_.resize(gold_mines_count);

			for (const auto& gold_mine : gold_mines_)
				gold_mine->update_from_packet(*packet);
		}
	}

	camera_position_text_.setString("x: " + std::to_string(static_cast<int>(camera_position_)));
	
	set_objects_screen_place();

	if (my_statue_->is_destroyed())
	{
		Army::play_in_attack_music(false);
		state_manager_.switch_state(lose_menu);
	}
	else if (enemy_statue_->is_destroyed())
	{
		Army::play_in_attack_music(false);
		state_manager_.switch_state(victory_menu);
	}
}

void PlayState::handle_input(Input& input, const sf::Time delta_time)
{
	if (input.mouse_left and pause_button_->check_mouse_pressed(input.mouse_position))
	{
		state_manager_.switch_state(pause);
		return;
	}

	int my_player_id = 0;
	if(client_handler == nullptr)
	{
		int player_id = 0;

		players_[player_id].handle_input(input, 0, delta_time);

		if(server_handler != nullptr)
		{
			auto clients_input_vector = server_handler->get_clients_input();
			for (auto& client_input_packet : clients_input_vector)
			{
				player_id++;
				if(client_input_packet)
				{
					Input client_input;
					client_input.read_from_packet(*client_input_packet);
					float client_camera_position_;
					*client_input_packet >> client_camera_position_;

					players_[player_id].handle_input(client_input, static_cast<int>(client_camera_position_ - camera_position_), delta_time);
				}
			}
		}
	}
	else
	{
		my_player_id = client_handler->get_id();
		sf::Packet input_packet;
		input.write_to_packet(input_packet);
		input_packet << camera_position_;
		client_handler->put_input_to_server(input_packet);
	}

	if (const auto controlled_unit_position = players_[my_player_id].get_controlled_unit_position(); controlled_unit_position)
	{
		const float window_width = static_cast<float>(sf::VideoMode::getDesktopMode().width);
		const float shift = (controlled_unit_position->x + 15 - window_width / 2 - camera_position_) / 15;
		move_camera(shift);
	}

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
	draw_queue.emplace(attributes_layer_0, &camera_position_text_);

	my_statue_->draw(draw_queue);
	enemy_statue_->draw(draw_queue);

	for (const auto& gold_mine : gold_mines_)
		gold_mine->draw(draw_queue);

	enemy_army_.draw(draw_queue);

	for (const auto& player : players_)
		player.draw(draw_queue);

	pause_button_->draw(draw_queue);
}

