#pragma once
#include <SFML/Graphics/RectangleShape.hpp>

class HealthBar
{
	const float& max_health_;
	const float& health_;
	const float max_size_;
	const sf::Vector2f shift_;
	sf::RectangleShape health_bar_;
public:
	HealthBar(const float& max_health, float& health, sf::Vector2f position, sf::Vector2f size = {70, 3}, sf::Vector2f shift = { -32, 50 }) :
	max_health_(max_health), health_(health), max_size_(size.x), shift_(shift)
	{
		set_position(position);
		health_bar_.setSize(size);
		health_bar_.setFillColor(sf::Color::Magenta);
	}
	void update()
	{
		float health_bar_size = health_ / max_health_ * max_size_;
		health_bar_.setSize({ health_bar_size, health_bar_.getSize().y });
	}
	void move(const sf::Vector2f offset)
	{
		health_bar_.move(offset);
	}
	void set_position(const sf::Vector2f position)
	{
		health_bar_.setPosition({ position.x + shift_.x, position.y + shift_.y });
	}
	void draw(sf::RenderWindow& window) const
	{
		window.draw(health_bar_);
	}
};
