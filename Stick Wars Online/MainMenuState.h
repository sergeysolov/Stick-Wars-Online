#pragma once
#include <SFML/Graphics.hpp>

#include "UserInterface.h"
#include "StateManager.h"

class StateManager;
class Button;

class MainMenuState : public BaseState
{
	StateManager& state_manager_;

	sf::RectangleShape background_;

	std::unique_ptr<Button> play_button_;
	std::unique_ptr<Button> exit_button_;
	
public:
	explicit MainMenuState(StateManager& state_manager);

	void update(sf::Time delta_time) override;
	void handle_input(Input& input, sf::Time delta_time) override;
	void draw(DrawQueue& draw_queue) override;
};

