#pragma once
#include <SFML/Graphics.hpp>
#include "DrawQueue.h"
#include "Input.h"

class BaseState
{
public:
	virtual void update(sf::Time delta_time) = 0;
	virtual void handle_input(Input& input, sf::Time delta_time) = 0;
	virtual void draw(DrawQueue& draw_queue) = 0;
};
