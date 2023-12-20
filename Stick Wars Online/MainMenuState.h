#pragma once
#include <SFML/Graphics.hpp>

#include "UserInterface.h"
#include "StateManager.h"
#include "ConnectionHandler.h"

class StateManager;
class Button;

class MainMenuState : public BaseState
{
	StateManager& state_manager_;

	sf::RectangleShape background_;

	std::unique_ptr<Button> play_single_player_button_;
	std::unique_ptr<Button> play_multi_player_button_;
	std::unique_ptr<Button> exit_button_;
	
public:
	explicit MainMenuState(StateManager& state_manager);

	void update(sf::Time delta_time) override;
	void handle_input(Input& input, sf::Time delta_time) override;
	void draw(DrawQueue& draw_queue) override;
};


class MultiplayerMenuState : public BaseState
{
	StateManager& state_manager_;

	std::unique_ptr<Button> back_button_;
	std::unique_ptr<Button> create_game_button_;
	std::unique_ptr<Button> connect_button_;
	std::unique_ptr<Button> start_game_button_;

	constexpr static int acceptance_to_start_game_num = 761;
public:
	explicit MultiplayerMenuState(StateManager& state_manager);

	void update(sf::Time delta_time) override;
	void handle_input(Input& input, sf::Time delta_time) override;
	void draw(DrawQueue& draw_queue) override;
};
