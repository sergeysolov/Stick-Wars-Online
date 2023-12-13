#include "UserInterface.h"

Button::Button(sf::Vector2f position, sf::Vector2f scale, ID id, TextureHolder& holder)
{
	sprite_.setTexture(holder.get_texture(id));
	sprite_.setPosition(position);
	sprite_.setScale(scale);
}

const sf::Sprite& Button::get_sprite() const
{
	return sprite_;
}

void Button::draw(sf::RenderWindow& window) const
{
	window.draw(sprite_);
}

bool Button::check_mouse_pressed(const sf::Vector2i mouse_position) const
{
	return sprite_.getGlobalBounds().contains(static_cast<float>(mouse_position.x), static_cast<float>(mouse_position.y));
}

void Button::press()
{
	pressed_ = true;
}

bool Button::is_pressed()
{
	const bool temp = pressed_;
	pressed_ = false;
	return temp;
}

UnitBuyButton::UnitBuyButton(ID unit_id, int unit_cost, int wait_time, sf::Vector2f position, sf::Vector2f scale, ID id, TextureHolder& holder, const sf::Font& font)
	: Button(position, scale, id, holder), wait_time_(wait_time), unit_cost_(unit_cost), unit_id_(unit_id)
{
	time_bar_.setPosition({ position.x + 3, position.y + 100 });
	time_bar_.setFillColor(sf::Color::Cyan);
	time_bar_.setSize({ 0, 0 });

	count_text_.setFont(font);
	count_text_.setFillColor(sf::Color::Black);
	count_text_.setPosition({ position.x + 10, position.y + 50 });

	gold_icon_.setTexture(holder.get_texture(gold));
	gold_icon_.setScale({ 0.04f, 0.04f });
	gold_icon_.setPosition({ position.x + 5, position.y + 10 });

	cost_text_.setFont(font);
	cost_text_.setString(std::to_string(unit_cost).c_str());
	cost_text_.setFillColor(sf::Color::Black);
	cost_text_.setPosition({ position.x + 40, position.y + 10 });
	cost_text_.setCharacterSize(20);
}

void UnitBuyButton::draw(sf::RenderWindow& window) const
{
	window.draw(sprite_);
	window.draw(time_bar_);
	window.draw(gold_icon_);
	window.draw(cost_text_);
	if (remaining_time_ != 0)
		window.draw(count_text_);
}

void UnitBuyButton::press()
{
	remaining_time_ += wait_time_;
	this->process_button(1);
}

int UnitBuyButton::get_unit_cost() const
{
	return unit_cost_;
}

ID UnitBuyButton::get_unit_id() const
{
	return unit_id_;
}

void UnitBuyButton::process_button(const int elapsed_time)
{
	if (remaining_time_ >= elapsed_time)
		remaining_time_ -= elapsed_time;
	else
		remaining_time_ = 0;

	const std::string temp_str = "x" + std::to_string(static_cast<int>(ceil(static_cast<float>(remaining_time_) / wait_time_)));
	count_text_.setString(temp_str.c_str());

	const int remaining_time_for_current_unit = remaining_time_ % wait_time_;
	time_bar_.setSize({ bar_size_.x * remaining_time_for_current_unit / wait_time_, bar_size_.y });
}


bool UserInterface::process_unit_buy_buttons(const sf::Vector2i mouse_position) const
{
	for (const auto& unit_button : unit_buy_buttons_)
	{
		if (unit_button->check_mouse_pressed(mouse_position) and money_ >= unit_button->get_unit_cost())
		{
			bool unit_added = false;
			if (unit_button->get_unit_id() == Miner::texture_id and spawn_queue_.get_free_places() >= Miner::places_requires)
			{
				spawn_queue_.put_unit(std::make_shared<Miner>(UnitBuyButton::spawn_point, texture_holder_, unit_button->get_unit_id()), Miner::wait_time);
				unit_added = true;
			}
			else if (unit_button->get_unit_id() == Swordsman::texture_id and spawn_queue_.get_free_places() >= Swordsman::places_requires)
			{
				spawn_queue_.put_unit(std::make_shared<Swordsman>(UnitBuyButton::spawn_point, texture_holder_, unit_button->get_unit_id()), Swordsman::wait_time);
				unit_added = true;
			}

			if (unit_added)
			{
				money_ -= unit_button->get_unit_cost();
				unit_button->press();
			}
			return true;
		}
	}
	return false;
}

UserInterface::UserInterface(TextureHolder& holder, int& money, const float& camera_position, Army& army, SpawnUnitQueue& spawn_queue) :
	money_(money), camera_position_(camera_position), army_(army), spawn_queue_(spawn_queue), texture_holder_(holder)
{
	gold_sprite_.setTexture(holder.get_texture(gold));
	gold_sprite_.setPosition({ 20, 20 });
	gold_sprite_.setScale({ 0.1, 0.1 });

	text_font_.loadFromFile("Images/fonts/textfont.ttf");
	money_count_text_.setFont(text_font_);
	money_count_text_.setPosition(20, 70);

	stick_man_.setTexture(holder.get_texture(stick_man));
	stick_man_.setScale({ 0.25, 0.25 });
	stick_man_.setPosition({ 35, 120 });

	army_count_text_.setFont(text_font_);
	army_count_text_.setPosition({ 25, 170 });

	camera_position_text_.setFont(text_font_);
	camera_position_text_.setPosition(1800, 10);

	unit_buy_buttons_.push_back(std::make_unique<UnitBuyButton>(my_miner, Miner::cost, Miner::wait_time, sf::Vector2f{ 130, 20 }, sf::Vector2f{ 0.15f, 0.15f },
		miner_buy_button, holder, text_font_));
	unit_buy_buttons_.push_back(std::make_unique<UnitBuyButton>(my_swordsman, Swordsman::cost, Swordsman::wait_time, sf::Vector2f{ 230, 20 }, sf::Vector2f{ 0.15f, 0.15f },
		swordsman_buy_button, holder, text_font_));

	defend_button_.reset(new Button({ 900.0f, 20.0f }, { 0.15f, 0.15f }, defend_button, holder));
	in_attack_button_.reset(new Button({ 1000.0f, 20.0f }, { 0.15f, 0.15f }, in_attack_button, holder));
}

bool UserInterface::process_left_mouse_button_press(const sf::Vector2i mouse_position) const
{
	const bool unit_buy_button_pressed = process_unit_buy_buttons(mouse_position);
	if(not unit_buy_button_pressed)
	{
		if (defend_button_->check_mouse_pressed(mouse_position))
		{
			army_.set_army_target(Army::defend);
			return true;
		}
		if (in_attack_button_->check_mouse_pressed(mouse_position))
		{
			army_.set_army_target(Army::attack);
			return true;
		}
	}
	return unit_buy_button_pressed;
}

void UserInterface::update(const sf::Time delta_time)
{
	// update army count text
	const std::string str = std::to_string(spawn_queue_.get_army_count()) + "/" + std::to_string(Army::army_max_size);
	army_count_text_.setString(str);

	//process units queue and units spawn
	for (const auto& unit_button : unit_buy_buttons_)
		if (auto id = spawn_queue_.get_front_unit_id(); id and unit_button->get_unit_id() == *id)
			unit_button->process_button(delta_time.asMilliseconds());

	camera_position_text_.setString("x: " + std::to_string(static_cast<int>(camera_position_)));

	money_count_text_.setString(std::to_string(money_));
}

void UserInterface::draw(sf::RenderWindow& window) const
{
	window.draw(gold_sprite_);
	window.draw(money_count_text_);

	for (const auto& unit_button : unit_buy_buttons_)
		unit_button->draw(window);

	in_attack_button_->draw(window);
	defend_button_->draw(window);

	window.draw(stick_man_);
	window.draw(army_count_text_);
	window.draw(camera_position_text_);
}
