#pragma once
#include <SFML/Graphics.hpp>

class Input
{
public:
	void process_events(sf::RenderWindow& window);
	bool a = false;
	bool d = false;
	bool w = false;
	bool s = false;
	bool k = false;
	bool left_arrow = false;
	bool right_arrow = false;
	bool space = false;
	bool shift = false;
	bool escape = false;
	bool mouse_left = false;

	sf::Vector2i mouse_position;
};

