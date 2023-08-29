#include <SFML/Graphics.hpp>
#include <iostream>
#include "Game.h"

int main()
{
	srand(time(0));
	Game game(1920, 1080, "Stick Wars Online");
	game.init();
	game.run();
	return 0;
}