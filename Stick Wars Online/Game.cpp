#include "Game.h"

Game::Game(const unsigned width, const unsigned height, const char* title)
	: main_window_(sf::VideoMode(width, height), title, sf::Style::Fullscreen), state_manager_(main_window_)
{
	UnitFactory::init();
}

int Game::run()
{
	// main_window_.setVerticalSyncEnabled(true);
	main_window_.setFramerateLimit(90);
	state_manager_.switch_state(main_menu);
	while (main_window_.isOpen())
	{
		const sf::Time delta_time = clock_.restart();

		input_.process_events(main_window_);

		state_manager_.handle_input(input_, delta_time);
		state_manager_.update(delta_time);

		draw();

		main_window_.display();
	}
	return 0;
}

void Game::draw()
{
	main_window_.clear();

	state_manager_.draw(draw_queue_);

	while (not draw_queue_.empty())
	{
		main_window_.draw(*draw_queue_.top().object);
		draw_queue_.pop();
	}
}