#pragma once
#include <functional>
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include "Input.h"
#include "DrawQueue.h"

#include "BaseState.h"
#include "PlayState.h"
#include "MainMenuState.h"
#include "GameOverState.h"

enum StateType
{
	to_exit,
	main_menu,
	play,
	pause,
	lose_menu,
	victory_menu
};

class StateManager
{
	template<class T>
	void register_state(StateType state);
	std::unordered_map<StateType, std::function<BaseState*()>> state_factory_;

	sf::RenderWindow& window_;

	std::vector<std::pair<StateType, std::unique_ptr<BaseState>>> states_;
public:
	StateManager(sf::RenderWindow& window);

	void switch_state(StateType state);

	void handle_input(Input& input, sf::Time delta_time) const;
	void update(sf::Time delta_time) const;
	void draw(DrawQueue& draw_queue) const;
};




