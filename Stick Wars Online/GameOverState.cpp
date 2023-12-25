#include "GameOverState.h"

void GameOverState::update(sf::Time delta_time)
{
	if (to_menu_button_->is_pressed())
	{
		if (client_handler != nullptr or server_handler != nullptr)
			state_manager_.switch_state(multiplayer_menu);
		else
			state_manager_.switch_state(main_menu);
	}
}

void GameOverState::handle_input(Input& input, sf::Time delta_time)
{
	if (to_menu_button_->check_mouse_pressed(input.mouse_position))
		to_menu_button_->press();
}

void GameOverState::draw(DrawQueue& draw_queue)
{
	draw_queue.emplace(attributes_layer_1, &background_);
	draw_queue.emplace(interface_layer_0, &text_);
	to_menu_button_->draw(draw_queue);
}

GameOverState::GameOverState(StateManager& state_manager, const std::string& text)
	: state_manager_(state_manager)
{
	background_.setFillColor(sf::Color{ 50, 120, 120 });
	background_.setPosition({ 420, 200 });
	background_.setSize({ 1000, 600 });

	text_.setFont(text_font);
	text_.setString(text);
	text_.setFillColor(sf::Color{ 60, 8, 60 });
	text_.setPosition({ 750, 250 });
	text_.setScale({ 2.5f,2.5f });

	to_menu_button_ = std::make_unique<Button>(sf::Vector2f{ 775, 625 }, sf::Vector2f{ 300, 90 }, sf::Color::Black);
	to_menu_button_->get_text().setFont(text_font);
	to_menu_button_->get_text().setString("Quit to menu");
	to_menu_button_->get_text().move({ 45, 25 });
}

VictoryState::VictoryState(StateManager& state_manager) : GameOverState(state_manager, "Victory!")
{

}

DefeatState::DefeatState(StateManager& state_manager) : GameOverState(state_manager, "Defeat!")
{
}

PauseState::PauseState(StateManager& state_manager) : GameOverState(state_manager, "Pause")
{
	back_to_game_button_ = std::make_unique<Button>(sf::Vector2f{ 775, 500 }, sf::Vector2f{ 300, 90 }, sf::Color::Black);
	back_to_game_button_->get_text().setFont(text_font);
	back_to_game_button_->get_text().setString("Back");
	back_to_game_button_->get_text().move({ 45, 25 });
}

void PauseState::update(const sf::Time delta_time)
{
	if (back_to_game_button_->is_pressed())
	{
		state_manager_.switch_state(play);
	}
	else
		GameOverState::update(delta_time);
}

void PauseState::handle_input(Input& input, const sf::Time delta_time)
{
	GameOverState::handle_input(input, delta_time);
	if (back_to_game_button_->check_mouse_pressed(input.mouse_position))
	{
		back_to_game_button_->press();
	}
}

void PauseState::draw(DrawQueue& draw_queue)
{
	GameOverState::draw(draw_queue);
	back_to_game_button_->draw(draw_queue);
}

