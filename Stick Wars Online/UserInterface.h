#pragma once
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include "TextureHolder.h"
#include "Army.h"

class Button
{
protected:
	sf::Sprite sprite_;
	bool pressed_ = false;
public:
	virtual ~Button() = default;
	Button(sf::Vector2f position, sf::Vector2f scale, ID id, TextureHolder& holder);
	const sf::Sprite& get_sprite() const;
	virtual void draw(sf::RenderWindow& window) const;
	bool check_mouse_pressed(sf::Vector2i mouse_position) const;
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
	sf::Text cost_text_;

	int unit_cost_;
	ID unit_id_;

public:
	inline static const sf::Vector2f spawn_point = { -100, 650 };

	UnitBuyButton(ID unit_id, int unit_cost, int wait_time, sf::Vector2f position, sf::Vector2f scale, ID id, TextureHolder& holder, const sf::Font& font);
	void draw(sf::RenderWindow& window) const override;
	int get_unit_cost() const;
	ID get_unit_id() const;
	void press() override;
	void process_button(int elapsed_time);
};


class UserInterface
{
	sf::Sprite gold_sprite_;
	sf::Sprite stick_man_;

	sf::Font text_font_;

	sf::Text army_count_text_;
	sf::Text money_count_text_;
	sf::Text camera_position_text_;

	std::vector<std::unique_ptr<UnitBuyButton>> unit_buy_buttons_;
	std::unique_ptr<Button> in_attack_button_ = nullptr;
	std::unique_ptr<Button> defend_button_ = nullptr;

	bool process_unit_buy_buttons(sf::Vector2i mouse_position) const;
public:

	UserInterface(TextureHolder& holder, int& money, const float& camera_position, Army& army, SpawnUnitQueue& spawn_queue);

	bool process_left_mouse_button_press(sf::Vector2i mouse_position) const;
	void update(sf::Time delta_time);
	void draw(sf::RenderWindow& window) const;

protected:
	int& money_;
	const float& camera_position_;
	Army& army_;
	SpawnUnitQueue& spawn_queue_;
	TextureHolder& texture_holder_;
};

