#pragma once
#include <SFML/Graphics/RectangleShape.hpp>

class HealthBar
{
	const float& max_health_;
	const float& health_;
	const float max_size_;
	const sf::Vector2f shift_;
	sf::RectangleShape health_bar_;
	sf::RectangleShape total_health_bar_;
public:
	inline const static sf::Vector2f unit_health_bar_size = { 70, 3 };
	inline const static sf::Vector2f statue_health_bar_size = { 100, 15 };

	inline const static sf::Vector2f unit_health_bar_shift = { -32, 50 };
	inline const static sf::Vector2f statue_health_bar_shift = {-32, 60};

	HealthBar(const float& max_health, const float& health, const sf::Vector2f position, const sf::Vector2f size, const sf::Vector2f shift) :
	max_health_(max_health), health_(health), max_size_(size.x), shift_(shift)
	{
		set_position(position);

		total_health_bar_.setSize(size);
		total_health_bar_.setFillColor(sf::Color::White);
		
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
		total_health_bar_.move(offset);
		health_bar_.move(offset);
	}
	void set_position(const sf::Vector2f position)
	{
		total_health_bar_.setPosition({ position.x + shift_.x, position.y + shift_.y });
		health_bar_.setPosition({ position.x + shift_.x, position.y + shift_.y });
	}
	void draw(sf::RenderWindow& window) const
	{
		if(health_ > 0)
		{
			window.draw(total_health_bar_);
			window.draw(health_bar_);
		}
	}
};
