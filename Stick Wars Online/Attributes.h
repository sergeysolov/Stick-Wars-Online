#pragma once
#include <SFML/Graphics.hpp>
#include "DrawQueue.h"

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
	inline const static sf::Vector2f statue_health_bar_shift = { -32, 60 };

	inline const static sf::Vector2f miner_gold_count_bar_shift = { -32, 35 };

	inline const static sf::Color health_bar_color = sf::Color{ 255, 0, 100 };
	inline const static sf::Color miner_gold_bar_color = sf::Color{ 210, 160, 30 };

	Bar(const T& max_value, const T& value, sf::Vector2f position, sf::Vector2f size, sf::Vector2f shift, sf::Color color);
	void update();
	void move(sf::Vector2f offset);
	void set_position(sf::Vector2f position);
	void draw(DrawQueue& queue) const;
};



template <typename T>
Bar<T>::Bar(const T& max_value, const T& value, sf::Vector2f position, sf::Vector2f size, sf::Vector2f shift,
	sf::Color color)
	:
	max_value_(max_value), value_(value), max_size_(size.x), shift_(shift)
{
	set_position(position);

	total_bar_.setSize(size);
	total_bar_.setFillColor(sf::Color::White);

	bar_.setSize(size);
	bar_.setFillColor(color);
}

template <typename T>
void Bar<T>::update()
{
	float bar_size = static_cast<float> (value_) / max_value_ * max_size_;
	bar_.setSize({ bar_size, bar_.getSize().y });
}

template <typename T>
void Bar<T>::move(const sf::Vector2f offset)
{
	total_bar_.move(offset);
	bar_.move(offset);
}

template <typename T>
void Bar<T>::set_position(const sf::Vector2f position)
{
	total_bar_.setPosition({ position.x + shift_.x, position.y + shift_.y });
	bar_.setPosition({ position.x + shift_.x, position.y + shift_.y });
}

template <typename T>
void Bar<T>::draw(DrawQueue& queue) const
{
	queue.emplace(attributes_layer_0, &total_bar_);
	queue.emplace(attributes_layer_1, &bar_);
}
