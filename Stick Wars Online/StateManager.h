#pragma once
#include <functional>
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include "Input.h"
#include "DrawQueue.h"

#include "BaseState.h"
#include "PlayState.h"

enum StateType
{
	main_menu,
	play,
	pause,
	lose_menu,
	victory_menu
};

class StateManager
{
	std::unordered_map<StateType, std::function<BaseState*()>> state_factory_;
	std::unique_ptr<BaseState> current_state_ = nullptr;

	template<class T>
	void register_state(const StateType& state);

public:
	StateManager();

	void switch_state(StateType state);

	void update(sf::Time delta_time) const;
	void draw(DrawQueue& draw_queue) const;
	void handle_input(Input& input, sf::Time delta_time) const;
};




