#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

class Input
{
public:
	void process_events(sf::RenderWindow& window);
	bool a = false;
	bool d = false;
	bool w = false;
	bool s = false;
	bool k = false;
	bool e = false;
	bool f = false;
	bool left_arrow = false;
	bool right_arrow = false;
	bool up_arrow = false;
	bool down_arrow = false;
	bool space = false;
	bool shift = false;
	bool escape = false;
	bool mouse_left = false;
	bool mouse_right = false;

	sf::Vector2i mouse_position;

	void write_to_packet(sf::Packet& packet) const;
	void read_from_packet(sf::Packet& packet);
};

