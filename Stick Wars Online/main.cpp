#include <SFML/Graphics.hpp>
#include <iostream>
#include "Game.h"

int main()
{
	text_font.loadFromFile("Images/fonts/textfont.ttf");

	Game game(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height, "Stick Wars Online");
	game.run();
	return 0;
}