#include "MainMenuState.h"

MainMenuState::MainMenuState(StateManager& state_manager) : state_manager_(state_manager)
{
	background_.setSize(static_cast<sf::Vector2f>(sf::Vector2u{ sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height }));
	background_.setTexture(&texture_holder.get_texture(intro));

	play_single_player_button_ = std::make_unique<Button>(sf::Vector2f{ 170, 200 }, sf::Vector2f{ 350, 100 }, sf::Color::Black);
	play_single_player_button_->set_text("Play singleplayer", text_font, { 50, 30 });

	play_multi_player_button_ = std::make_unique<Button>(sf::Vector2f{ 170, 350 }, sf::Vector2f{ 350, 100 }, sf::Color::Black);
	play_multi_player_button_->set_text("Play multiplayer", text_font, { 50, 30 });

	back_button_ = std::make_unique<Button>(sf::Vector2f{ 170, 500 }, sf::Vector2f{ 250, 100 }, sf::Color::Black);
	back_button_->set_text("Back", text_font, { 90, 30 });
	back_button_->set_visible(false);

	create_game_button_ = std::make_unique<Button>(sf::Vector2f{ 500, 200 }, sf::Vector2f{ 350, 100 }, sf::Color::Black);
	create_game_button_->set_text("Create game", text_font, { 50, 30 });
	create_game_button_->set_visible(false);

	connect_button_ = std::make_unique<Button>(sf::Vector2f{ 500, 350 }, sf::Vector2f{ 350, 100 }, sf::Color::Black);
	connect_button_->set_text("Connect", text_font, { 50, 30 });
	connect_button_->set_visible(false);

	exit_button_ = std::make_unique<Button>(sf::Vector2f{ 170, 650 }, sf::Vector2f{ 250, 100 }, sf::Color::Black);
	exit_button_->set_text("Exit", text_font, { 90, 30 });
}

void MainMenuState::update(sf::Time delta_time)
{
	if (play_single_player_button_->is_pressed())
		state_manager_.switch_state(play);
	else if(play_multi_player_button_->is_pressed())
	{
		play_single_player_button_->set_visible(false);
		play_multi_player_button_->set_visible(false);
		create_game_button_->set_visible(true);
		connect_button_->set_visible(true);
		back_button_->set_visible(true);
	}
	else if(back_button_->is_pressed())
	{
		play_single_player_button_->set_visible(true);
		play_multi_player_button_->set_visible(true);
		create_game_button_->set_visible(false);
		connect_button_->set_visible(false);
		back_button_->set_visible(false);
	}
	else if(create_game_button_->is_pressed())
	{
		if (server_handler == nullptr)
			server_handler = std::make_unique<ServerConnectionHandler>();
		client_handler.reset();
		server_handler->listen_for_client_connection();
	}
	else if(connect_button_->is_pressed())
	{
		if (client_handler == nullptr)
			client_handler = std::make_unique<ClientConnectionHandler>();
		server_handler.reset();
		client_handler->connect();
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
	else if (back_button_->check_mouse_pressed(input.mouse_position))
		back_button_->press();
	else if (exit_button_->check_mouse_pressed(input.mouse_position))
		exit_button_->press();
	else if (create_game_button_->check_mouse_pressed(input.mouse_position))
		create_game_button_->press();
	else if (connect_button_->check_mouse_pressed(input.mouse_position))
		connect_button_->press();
}

void MainMenuState::draw(DrawQueue& draw_queue)
{
	draw_queue.emplace(background, &background_);
	play_single_player_button_->draw(draw_queue);
	play_multi_player_button_->draw(draw_queue);
	create_game_button_->draw(draw_queue);
	connect_button_->draw(draw_queue);
	back_button_->draw(draw_queue);
	exit_button_->draw(draw_queue);
}
