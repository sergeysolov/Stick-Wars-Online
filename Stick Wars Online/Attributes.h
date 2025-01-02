#pragma once
#include <SFML/Graphics.hpp>
#include "DrawQueue.h"
#include "TextureHolder.h"

template <typename T>
class Bar
{
protected:
	const T max_value_;
	const T& value_;
	const float max_size_;
	const sf::Vector2f shift_;
	sf::RectangleShape bar_;
	sf::RectangleShape total_bar_;
public:
	inline const static sf::Vector2f unit_bar_size = { 70, 3 };
	inline const static sf::Vector2f statue_health_bar_size = { 100, 15 };

	inline const static sf::Vector2f unit_health_bar_offset = { 0, 20 };
	inline const static sf::Vector2f unit_second_attribute_bar_offset = { 0, 5 };

	inline const static sf::Vector2f statue_health_bar_offset = { -32 + 50, 65 };

	inline const static sf::Color health_bar_color = sf::Color{ 255, 0, 100 };
	inline const static sf::Color miner_gold_bar_color = sf::Color{ 210, 160, 30 };
	inline const static sf::Color magikill_cooldown_time_bar_color = sf::Color{60, 160, 250};

	Bar(T max_value, const T& value, sf::Vector2f position, sf::Vector2f size, sf::Vector2f shift, sf::Color color);
	void update();
	void move(sf::Vector2f offset);
	void set_position(sf::Vector2f position);
	void set_scale(sf::Vector2f scale_factor);
	void draw(DrawQueue& queue) const;
};



template <typename T>
Bar<T>::Bar(const T max_value, const T& value, sf::Vector2f position, sf::Vector2f size, sf::Vector2f shift,
	sf::Color color)
	:
	max_value_(max_value), value_(value), max_size_(size.x), shift_(shift)
{
	total_bar_.setSize(size);
	total_bar_.setOrigin({ size.x / 2, size.y / 2 });
	total_bar_.setFillColor(sf::Color::White);

	bar_.setSize(size);
	bar_.setOrigin({ size.x / 2, size.y / 2 });
	bar_.setFillColor(color);

	set_position(position);
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
void Bar<T>::set_scale(const sf::Vector2f scale_factor)
{
	total_bar_.setScale(scale_factor);
	bar_.setScale(scale_factor);
}

template <typename T>
void Bar<T>::draw(DrawQueue& queue) const
{
	queue.emplace(attributes_layer_0, &total_bar_);
	queue.emplace(attributes_layer_1, &bar_);
}


class Aim
{
protected:
	sf::Sprite sprite_;
	sf::Vector2f position_;
	sf::Vector2f scale_factor_;
	sf::Vector2f shift_;
	const sf::Vector2f static_shift_;
	float angle_ = 0.f;

	void update_sprite_position();
public:
	static constexpr float min_bow_angle = -1.5f;
	static constexpr float max_bow_angle = 0.6f;

	inline static const auto archer_scale = sf::Vector2f{ 0.08f, 0.08f };
	inline static const auto archer_shift = sf::Vector2f{ 1400, 1400};
	inline static const auto archer_static_shift = sf::Vector2f{ -300, 400 };

	Aim(sf::Vector2f position, sf::Vector2f scale_factor, sf::Vector2f static_shift, sf::Vector2f shift);

	sf::Vector2f get_direction() const;
	float get_angle() const;

	void set_direction(const int direction);
	void move(const sf::Vector2f offset, const float angle);
	void set_position(sf::Vector2f position);
	void set_scale(sf::Vector2f scale_factor);
	void draw(DrawQueue& queue) const;
};
