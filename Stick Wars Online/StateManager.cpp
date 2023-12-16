#include "StateManager.h"

template <class T>
void StateManager::register_state(const StateType& state)
{
	state_factory_[state] = []() -> BaseState*
		{
			return new T();
		};
}

StateManager::StateManager()
{
	register_state<PlayState>(play);
}

void StateManager::switch_state(const StateType state)
{
	current_state_.reset(state_factory_[state]());
}

void StateManager::update(const sf::Time delta_time) const
{
    current_state_->update(delta_time);
}

void StateManager::draw(DrawQueue& draw_queue) const
{
    current_state_->draw(draw_queue);
}

void StateManager::handle_input(Input& input, const sf::Time delta_time) const
{
    current_state_->handle_input(input, delta_time);
}
