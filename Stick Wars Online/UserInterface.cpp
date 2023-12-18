#include "UserInterface.h"

Button::Button(const sf::Vector2f position, const sf::Vector2f sprite_scale, const texture_ID sprite_id)
{
	rectangle_.setPosition(position);
	rectangle_.setSize(static_cast<sf::Vector2f>(texture_holder.get_texture(sprite_id).getSize()));
	rectangle_.setTexture(&texture_holder.get_texture(sprite_id));
	rectangle_.setScale(sprite_scale);
	text_.setPosition(position);
}

Button::Button(const sf::Vector2f position, const sf::Vector2f rectangle_size, const sf::Color rectangle_color)
{
	rectangle_.setPosition(position);
	rectangle_.setSize(rectangle_size);
	rectangle_.setFillColor(rectangle_color);
	text_.setPosition(position);
}

sf::Text& Button::get_text()
{
	return text_;
}


void Button::draw(DrawQueue& queue) const
{
	queue.emplace(interface_layer_0, &rectangle_);
	queue.emplace(interface_layer_1, &text_);
}

bool Button::check_mouse_pressed(const sf::Vector2i mouse_position) const
{
	return rectangle_.getGlobalBounds().contains(static_cast<float>(mouse_position.x), static_cast<float>(mouse_position.y));
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


UnitBuyButton::UnitBuyButton(std::function<Unit*()> unit_creation, int unit_cost, int wait_time, sf::Vector2f position, sf::Vector2f scale, texture_ID id)
	: Button(position, scale, id), wait_time_(wait_time), unit_cost_(unit_cost), unit_creation_(std::move(unit_creation))
{
	time_bar_.setPosition({ position.x + 3, position.y + 100 });
	time_bar_.setFillColor(sf::Color::Cyan);
	time_bar_.setSize({ 0, 0 });

	count_text_.setFont(text_font);
	count_text_.setFillColor(sf::Color::Black);
	count_text_.setPosition({ position.x + 10, position.y + 50 });

	gold_icon_.setTexture(texture_holder.get_texture(gold));
	gold_icon_.setScale({ 0.04f, 0.04f });
	gold_icon_.setPosition({ position.x + 5, position.y + 10 });

	text_.setFont(text_font);
	text_.setString(std::to_string(unit_cost).c_str());
	text_.setFillColor(sf::Color::Black);
	text_.setPosition({ position.x + 40, position.y + 10 });
	text_.setCharacterSize(20);
}

void UnitBuyButton::draw(DrawQueue& queue) const
{
	Button::draw(queue);
	queue.emplace(interface_layer_0, &time_bar_);
	queue.emplace(interface_layer_1, &gold_icon_);

	if (remaining_time_ != 0)
		queue.emplace(interface_layer_1, &count_text_);
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

Unit* UnitBuyButton::create_unit() const
{
	return unit_creation_();
}


bool UserInterface::process_unit_buy_buttons(const sf::Vector2i mouse_position) const
{
	for (const auto& unit_button : unit_buy_buttons_)
	{
		if (unit_button->check_mouse_pressed(mouse_position))
		{
			if(game_.get_money_count() >= unit_button->get_unit_cost())
			{
				const auto unit = std::shared_ptr<Unit>(unit_button->create_unit());
				if (game_.get_SpawnQueue().get_free_places() >= unit->get_places_requires())
				{
					game_.get_SpawnQueue().put_unit(unit, unit->get_wait_time());
					game_.get_money_count() -= unit_button->get_unit_cost();
					unit_button->press();
				}
			}
			return true;
		}
	}
	return false;
}

UserInterface::UserInterface(IPlayState& play_state) : game_(play_state)
{
	gold_sprite_.setTexture(texture_holder.get_texture(gold));
	gold_sprite_.setPosition({ 20, 20 });
	gold_sprite_.setScale({ 0.1, 0.1 });


	money_count_text_.setFont(text_font);
	money_count_text_.setPosition(20, 70);

	stick_man_.setTexture(texture_holder.get_texture(stick_man));
	stick_man_.setScale({ 0.25, 0.25 });
	stick_man_.setPosition({ 35, 120 });

	army_count_text_.setFont(text_font);
	army_count_text_.setPosition({ 25, 170 });

	camera_position_text_.setFont(text_font);
	camera_position_text_.setPosition(1800, 10);

	auto miner_creation = [] { return new Miner(UnitBuyButton::spawn_point, my_miner); };
	unit_buy_buttons_.push_back(std::make_unique<UnitBuyButton>(miner_creation, Miner::cost, Miner::wait_time, sf::Vector2f{ 130, 20 }, sf::Vector2f{ 0.15f, 0.15f },
		miner_buy_button));

	auto swardsman_creation = [] { return new Swordsman(UnitBuyButton::spawn_point, my_swordsman); };
	unit_buy_buttons_.push_back(std::make_unique<UnitBuyButton>(swardsman_creation, Swordsman::cost, Swordsman::wait_time, sf::Vector2f{ 230, 20 }, sf::Vector2f{ 0.15f, 0.15f },
		swordsman_buy_button));

	defend_button_.reset(new Button({ 900.0f, 20.0f }, { 0.15f, 0.15f }, defend_button));
	in_attack_button_.reset(new Button({ 1000.0f, 20.0f }, { 0.15f, 0.15f }, in_attack_button));

	pause_button_ = std::make_unique<Button>(sf::Vector2f{ 1600.f, 20.f }, sf::Vector2f{ 0.15f, 0.15f }, pause_button);
}

bool UserInterface::process_left_mouse_button_press(const sf::Vector2i mouse_position) const
{
	const bool unit_buy_button_pressed = process_unit_buy_buttons(mouse_position);
	if(not unit_buy_button_pressed)
	{
		if (defend_button_->check_mouse_pressed(mouse_position))
		{
			game_.get_Army().set_army_target(Army::defend);
			return true;
		}
		if (in_attack_button_->check_mouse_pressed(mouse_position))
		{
			game_.get_Army().set_army_target(Army::attack);
			return true;
		}
		if(pause_button_->check_mouse_pressed(mouse_position))
		{
			game_.get_StateManager().switch_state(pause);
			return true;
		}
	}
	return unit_buy_button_pressed;
}

void UserInterface::update(const sf::Time delta_time)
{
	// update army count text
	const std::string str = std::to_string(game_.get_SpawnQueue().get_army_count()) + "/" + std::to_string(Army::army_max_size);
	army_count_text_.setString(str);

	//process units queue and units spawn
	if (const auto id = game_.get_SpawnQueue().get_front_unit_id(); id)
		unit_buy_buttons_[*id]->process_button(delta_time.asMilliseconds());

	camera_position_text_.setString("x: " + std::to_string(static_cast<int>(game_.get_camera_position())));

	money_count_text_.setString(std::to_string(game_.get_money_count()));
}

void UserInterface::draw(DrawQueue& queue) const
{
	queue.emplace(interface_layer_0, &gold_sprite_);
	queue.emplace(interface_layer_0, &money_count_text_);

	for (const auto& unit_button : unit_buy_buttons_)
		unit_button->draw(queue);

	in_attack_button_->draw(queue);
	defend_button_->draw(queue);
	pause_button_->draw(queue);

	queue.emplace(interface_layer_0, &stick_man_);
	queue.emplace(interface_layer_0, &army_count_text_);
	queue.emplace(interface_layer_0, &camera_position_text_);
}
