#include "Attributes.h"
#include <algorithm>

void Aim::update_sprite_position()
{
	sprite_.setPosition({
		position_.x + (shift_.x * std::cos(angle_) + static_shift_.x) * scale_factor_.x,
		position_.y + (shift_.y * std::sin(angle_) + static_shift_.y) * scale_factor_.y});
}

Aim::Aim(
	sf::Vector2f position,
	sf::Vector2f scale_factor,
	sf::Vector2f static_shift,
	sf::Vector2f shift)
	: sprite_(texture_holder.get_texture(aim))
	, position_(position)
	, scale_factor_(scale_factor)
	, static_shift_(static_shift)
	, shift_(shift)
{
	sprite_.setScale(scale_factor);
	update_sprite_position();
}

sf::Vector2f Aim::get_direction() const
{
	return {std::cos(angle_), std::sin(angle_)};
}

float Aim::get_angle() const
{
	return angle_;
}

void Aim::set_direction(const int direction)
{
	shift_.x = abs(shift_.x) * direction;
}

void Aim::move(const sf::Vector2f offset, const float delta_angle)
{
	position_ += offset;
	angle_ = std::clamp(angle_ + delta_angle, min_bow_angle, max_bow_angle);
	update_sprite_position();
}

void Aim::set_position(sf::Vector2f position)
{
	position_ = position;
	update_sprite_position();
}

void Aim::set_scale(sf::Vector2f scale_factor)
{
	scale_factor_ = scale_factor;
	sprite_.setScale(scale_factor_);
}

void Aim::draw(DrawQueue& queue) const
{
	queue.emplace(attributes_layer_0, &sprite_);
}

Counter::Counter(const int& value, const std::optional<int> max_value, const sf::Vector2f shift)
	: value_(value)
	, max_value_(max_value)
	, shift_(shift)
{
	text_.setFont(text_font);
}

void Counter::update()
{
	std::string text = std::to_string(value_);
	if (max_value_.has_value()) {
		text += "/" + std::to_string(max_value_.value());
	}
	text_.setString(text);
}

void Counter::move(const sf::Vector2f offset)
{
	text_.move(offset);
}

void Counter::set_position(const sf::Vector2f position)
{
	text_.setPosition({ position.x + shift_.x * text_.getScale().x, position.y + shift_.y * text_.getScale().y });
}

void Counter::set_scale(const sf::Vector2f scale_factor)
{
	text_.setScale(scale_factor);
}

void Counter::draw(DrawQueue& queue) const
{
	queue.emplace(attributes_layer_1, &text_);
}
