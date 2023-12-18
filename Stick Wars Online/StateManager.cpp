#include "StateManager.h"
#include <ranges>

template <class T>
void StateManager::register_state(const StateType state)
{
	state_factory_[state] = [this]() -> BaseState*
		{
			return new T(*this);
		};
}

StateManager::StateManager(sf::RenderWindow& window) : window_(window)
{
	register_state<PlayState>(play);
	register_state<MainMenuState>(main_menu);
	register_state<LoseState>(lose_menu);
	register_state<VictoryState>(victory_menu);
	register_state<PauseState>(pause);
}

void StateManager::switch_state(const StateType state)
{
	if(state == to_exit)
	{
		window_.close();
		return;
	}
	const auto find_res = std::ranges::find_if(states_, [=](const auto& el) {return el.first == state; });
	if (find_res != states_.end())
	{
		while (states_.back().first != state)
			states_.pop_back();
	}
	else
		states_.emplace_back(state, state_factory_[state]());
}

void StateManager::update(const sf::Time delta_time) const
{
	states_.back().second->update(delta_time);
}

void StateManager::draw(DrawQueue& draw_queue) const
{
	for (const auto& [type, state] : states_ | std::ranges::views::reverse)
	{
		state->draw(draw_queue);
		if(type == play)
			break;
	}
}

void StateManager::handle_input(Input& input, const sf::Time delta_time) const
{
	states_.back().second->handle_input(input, delta_time);
}
