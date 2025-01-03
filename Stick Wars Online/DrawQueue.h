#pragma once
#include <queue>
#include <SFML/Graphics/Sprite.hpp>

enum DrawPriority
{
	background = 0,
	map_object = 10000,
	dead_units = 20000,
	arrows = 25000,
	alive_units = 30000,
	attributes_layer_0 = 40000,
	attributes_layer_1 = 50000,
	attributes_layer_2 = 60000,
	interface_layer_0 = 70000,
	interface_layer_1 = 80000,
};

struct DrawQueueItem
{
	DrawPriority priority;
	const sf::Drawable* object;

	bool operator<(const DrawQueueItem& other) const {
		return priority > other.priority;
	}
};

using DrawQueue = std::priority_queue<DrawQueueItem>;