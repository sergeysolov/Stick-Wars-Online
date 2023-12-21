#include "MainMenuState.h"

MainMenuState::MainMenuState(StateManager& state_manager) : state_manager_(state_manager)
{
	server_handler.reset();
	client_handler.reset();

	background_.setSize(static_cast<sf::Vector2f>(sf::Vector2u{ sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height }));
	background_.setTexture(&texture_holder.get_texture(intro));

	play_single_player_button_ = std::make_unique<Button>(sf::Vector2f{ 170, 200 }, sf::Vector2f{ 350, 100 }, sf::Color::Black);
	play_single_player_button_->set_text("Play singleplayer", text_font, { 50, 30 });

	play_multi_player_button_ = std::make_unique<Button>(sf::Vector2f{ 170, 350 }, sf::Vector2f{ 350, 100 }, sf::Color::Black);
	play_multi_player_button_->set_text("Play multiplayer", text_font, { 50, 30 });

	exit_button_ = std::make_unique<Button>(sf::Vector2f{ 170, 650 }, sf::Vector2f{ 250, 100 }, sf::Color::Black);
	exit_button_->set_text("Exit", text_font, { 90, 30 });
}

void MainMenuState::update(sf::Time delta_time)
{
	play_single_player_button_->set_visible(true);
	play_multi_player_button_->set_visible(true);
	exit_button_->set_visible(true);

	if (play_single_player_button_->is_pressed())
		state_manager_.switch_state(play);
	else if (play_multi_player_button_->is_pressed())
	{
		play_single_player_button_->set_visible(false);
		play_multi_player_button_->set_visible(false);
		exit_button_->set_visible(false);
		state_manager_.switch_state(multiplayer_menu);
	}
	if (exit_button_->is_pressed())
		state_manager_.switch_state(to_exit);
}

void MainMenuState::handle_input(Input& input, sf::Time delta_time)
{
	if(not input.mouse_left)
		return;

	if (play_single_player_button_->check_mouse_pressed(input.mouse_position))
		play_single_player_button_->press();
	else if (play_multi_player_button_->check_mouse_pressed(input.mouse_position))
		play_multi_player_button_->press();
	else if (exit_button_->check_mouse_pressed(input.mouse_position))
		exit_button_->press();
}

void MainMenuState::draw(DrawQueue& draw_queue)
{
	draw_queue.emplace(background, &background_);
	play_single_player_button_->draw(draw_queue);
	play_multi_player_button_->draw(draw_queue);
	exit_button_->draw(draw_queue);
}

MultiplayerMenuState::MultiplayerMenuState(StateManager& state_manager) : state_manager_(state_manager)
{
	back_button_ = std::make_unique<Button>(sf::Vector2f{ 170, 500 }, sf::Vector2f{ 250, 100 }, sf::Color::Black);
	back_button_->set_text("Back", text_font, { 90, 30 });

	create_game_button_ = std::make_unique<Button>(sf::Vector2f{ 500, 200 }, sf::Vector2f{ 350, 100 }, sf::Color::Black);
	create_game_button_->set_text("Create game", text_font, { 50, 30 });
	

	start_game_button_ = std::make_unique<Button>(sf::Vector2f{ 170, 200 }, sf::Vector2f{ 250, 100 }, sf::Color::Black);
	start_game_button_->set_text("Play", text_font, { 90, 30 });
	start_game_button_->set_visible(false);

	connect_button_ = std::make_unique<Button>(sf::Vector2f{ 500, 350 }, sf::Vector2f{ 350, 100 }, sf::Color::Black);
	connect_button_->set_text("Connect", text_font, { 50, 30 });
}

void MultiplayerMenuState::update(sf::Time delta_time)
{
	if(start_game_button_->is_pressed())
	{
		sf::Packet acceptance_to_start_game;
		acceptance_to_start_game << acceptance_to_start_game_num;
		for (const auto& client : server_handler->get_connections())
			client.get_socket().send(acceptance_to_start_game);
		server_handler->stop_listen();
		state_manager_.switch_state(play);
	}

	if (back_button_->is_pressed())
	{
		server_handler.reset();
		client_handler.reset();
		state_manager_.switch_state(main_menu);
	}
	else if (create_game_button_->is_pressed())
	{
		if (server_handler == nullptr)
			server_handler = std::make_unique<ServerConnectionHandler>();
		client_handler.reset();
		server_handler->read_player_name();
		server_handler->listen_for_client_connection();
	}
	else if (connect_button_->is_pressed())
	{
		if (client_handler == nullptr)
			client_handler = std::make_unique<ClientConnectionHandler>();
		server_handler.reset();
		client_handler->connect();
	}

	if (server_handler != nullptr)
		start_game_button_->set_visible(not server_handler->get_connections().empty());
	else if(client_handler != nullptr and client_handler->get_id() > 0)
	{
		client_handler->get_server().get_socket().setBlocking(false);
		sf::Packet acceptance_to_start_game;
		if(client_handler->get_server().get_socket().receive(acceptance_to_start_game) == sf::Socket::Done)
		{
			client_handler->get_server().get_socket().setBlocking(true);
			int num; acceptance_to_start_game >> num;
			if (num == acceptance_to_start_game_num)
				state_manager_.switch_state(play);
		}
	}
}

void MultiplayerMenuState::handle_input(Input& input, sf::Time delta_time)
{
	if(not input.mouse_left)
		return;

	if (back_button_->check_mouse_pressed(input.mouse_position))
		back_button_->press();
	else if (create_game_button_->check_mouse_pressed(input.mouse_position))
		create_game_button_->press();
	else if (connect_button_->check_mouse_pressed(input.mouse_position))
		connect_button_->press();
	else if (start_game_button_->check_mouse_pressed(input.mouse_position))
		start_game_button_->press();
}

void MultiplayerMenuState::draw(DrawQueue& draw_queue)
{
	start_game_button_->draw(draw_queue);
	create_game_button_->draw(draw_queue);
	connect_button_->draw(draw_queue);
	back_button_->draw(draw_queue);
}
