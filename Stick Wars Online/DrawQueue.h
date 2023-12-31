#pragma once
#include <queue>
#include <SFML/Graphics/Sprite.hpp>

enum DrawPriority
{
	background,
	map_object,
	dead_units,
	alive_units,
	attributes_layer_0,
	attributes_layer_1,
	attributes_layer_2,
	interface_layer_0,
	interface_layer_1,
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