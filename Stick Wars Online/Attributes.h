#pragma once
#include <SFML/Graphics/RectangleShape.hpp>

template <typename  T>
class Bar
{
protected:
	const T& max_value_;
	const T& value_;
	const float max_size_;
	const sf::Vector2f shift_;
	sf::RectangleShape bar_;
	sf::RectangleShape total_bar_;
public:
	inline const static sf::Vector2f unit_health_bar_size = { 70, 3 };
	inline const static sf::Vector2f statue_health_bar_size = { 100, 15 };

	inline const static sf::Vector2f unit_health_bar_shift = { -32, 50 };
	inline const static sf::Vector2f statue_health_bar_shift = {-32, 60};

	inline const static sf::Vector2f miner_gold_count_bar_shift = { -32, 35 };

	inline const static sf::Color health_bar_color = sf::Color{255, 0, 100};
	inline const static sf::Color miner_gold_bat_color = sf::Color{210, 160, 30};

	Bar(const T& max_value, const T& value, const sf::Vector2f position, const sf::Vector2f size, const sf::Vector2f shift, const sf::Color color) :
	max_value_(max_value), value_(value), max_size_(size.x), shift_(shift)
	{
		set_position(position);

		total_bar_.setSize(size);
		total_bar_.setFillColor(sf::Color::White);
		
		bar_.setSize(size);
		bar_.setFillColor(color);
	}
	void update()
	{
		float bar_size = static_cast<float> (value_) / max_value_ * max_size_;
		bar_.setSize({ bar_size, bar_.getSize().y });
	}
	void move(const sf::Vector2f offset)
	{
		total_bar_.move(offset);
		bar_.move(offset);
	}
	void set_position(const sf::Vector2f position)
	{
		total_bar_.setPosition({ position.x + shift_.x, position.y + shift_.y });
		bar_.setPosition({ position.x + shift_.x, position.y + shift_.y });
	}
	void draw(sf::RenderWindow& window) const
	{
		window.draw(total_bar_);
		window.draw(bar_);
	}
};
