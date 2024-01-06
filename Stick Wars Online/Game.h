#pragma once
#include <queue>
#include "DrawQueue.h"
#include "Input.h"
#include "StateManager.h"

class Game
{
	sf::RenderWindow main_window_;
	DrawQueue draw_queue_;

	StateManager state_manager_;

	Input input_;

	sf::Clock clock_;

	void draw();
public:
	Game(unsigned width, unsigned height, const char* title);
	int run();
};
