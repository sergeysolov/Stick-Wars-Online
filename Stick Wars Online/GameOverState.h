#pragma once
#include "BaseState.h"
#include "StateManager.h"

class StateManager;
class Button;

class GameOverState : public BaseState
{
protected:
	StateManager& state_manager_;

	sf::RectangleShape background_;
	std::unique_ptr<Button> to_menu_button_;
	sf::Text text_;
public:
	explicit GameOverState(StateManager& state_manager, const std::string& text);
	void update(sf::Time delta_time) override;
	void handle_input(Input& input, sf::Time delta_time) override;
	void draw(DrawQueue& draw_queue) override;
};



class VictoryState final : public GameOverState
{
public:
	explicit VictoryState(StateManager& state_manager);
};


class DefeatState final : public GameOverState
{
public:
	explicit DefeatState(StateManager& state_manager);
};


class PauseState final : public GameOverState
{
	std::unique_ptr<Button> back_to_game_button_;
public:
	explicit PauseState(StateManager& state_manager);

	void update(sf::Time delta_time) override;
	void handle_input(Input& input, sf::Time delta_time) override;
	void draw(DrawQueue& draw_queue) override;
};