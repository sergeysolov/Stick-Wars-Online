#pragma once
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include "TextureHolder.h"
#include "Army.h"
#include "PlayState.h"

class Button
{
protected:
	sf::RectangleShape rectangle_;
	sf::Text text_;
	bool pressed_ = false;
	bool visible_ = true;
public:
	virtual ~Button() = default;

	Button(sf::Vector2f position, sf::Vector2f sprite_scale, texture_ID sprite_id);
	Button(sf::Vector2f position, sf::Vector2f rectangle_size, sf::Color rectangle_color);

	sf::Text& get_text();
	void set_text(const std::string& text, const sf::Font& font, sf::Vector2f offset);
	void set_visible(bool visible);
	virtual void draw(DrawQueue& queue) const;
	[[nodiscard]] virtual bool check_mouse_pressed(sf::Vector2i mouse_position) const;
	virtual void press();
	bool is_pressed();
};


class UnitBuyButton : public Button
{
	sf::RectangleShape time_bar_;
	const sf::Vector2f bar_size_ = { 85, 5 };

	int remaining_time_ = 0;
	const int wait_time_;

	sf::Text count_text_;
	sf::Sprite gold_icon_;

	int unit_cost_;

public:

	UnitBuyButton(int unit_cost, int wait_time, sf::Vector2f position, sf::Vector2f scale, texture_ID id);
	void draw(DrawQueue& queue) const override;
	void press() override;
	void process_button(int elapsed_time);
	int get_unit_cost() const;
};

class StateManager;

class UserInterface
{
	sf::Sprite gold_sprite_;
	sf::Sprite stick_man_;

	sf::Text army_count_text_;
	sf::Text money_count_text_;

	std::vector<std::unique_ptr<UnitBuyButton>> unit_buy_buttons_;
	std::unique_ptr<Button> in_attack_button_;
	std::unique_ptr<Button> defend_button_;

public:
	UserInterface();

	void update(int money_count, int army_count, std::optional<int> unit_queue_id, sf::Time delta_time);
	void draw(DrawQueue& queue) const;

	std::vector<std::unique_ptr<UnitBuyButton>>& get_unit_buy_buttons();
	std::unique_ptr<Button>& get_in_attack_button();
	std::unique_ptr<Button>& get_defend_button();
};

