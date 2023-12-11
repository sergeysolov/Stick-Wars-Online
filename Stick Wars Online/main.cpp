#include <SFML/Graphics.hpp>
#include <iostream>
#include "Game.h"

int main()
{
	Game game(1920, 1080, "Stick Wars Online");
	game.run();
	return 0;
}