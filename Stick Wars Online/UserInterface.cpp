#include "UserInterface.h"

#include <ranges>

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

void Button::set_text(const std::string& text, const sf::Font& font, const sf::Vector2f offset)
{
	text_.setFont(font);
	text_.setString(text);
	text_.move(offset);
}

void Button::set_visible(const bool visible)
{
	visible_ = visible;
}

void Button::draw(DrawQueue& queue) const
{
	if(visible_)
	{
		queue.emplace(interface_layer_0, &rectangle_);
		queue.emplace(interface_layer_1, &text_);
	}
}

bool Button::check_mouse_pressed(const sf::Vector2i mouse_position) const
{
	return visible_ and rectangle_.getGlobalBounds().contains(static_cast<float>(mouse_position.x), static_cast<float>(mouse_position.y));
}

void Button::press_left()
{
	pressed_ = true;
}

bool Button::is_pressed()
{
	const bool temp = pressed_;
	pressed_ = false;
	return temp;
}

void Button::highlight(const bool is_highlight, const sf::Color color)
{
	if (is_highlight)
	{
		rectangle_.setOutlineThickness(highlight_thickness);
		rectangle_.setOutlineColor(color);
	}
	else
		rectangle_.setOutlineThickness(0);
}


UnitBuyButton::UnitBuyButton(int unit_cost, int wait_time, sf::Vector2f position, sf::Vector2f scale, texture_ID id)
	: Button(position, scale, id), wait_time_(wait_time), unit_cost_(unit_cost)
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

	if (remaining_time_ > 30)
		queue.emplace(interface_layer_1, &count_text_);
}

void UnitBuyButton::press_left()
{
	remaining_time_ += wait_time_;
	this->process(1);
}

void UnitBuyButton::press_right()
{
	remaining_time_ = std::max(remaining_time_ - wait_time_, 0);
	this->process(1);
}

int UnitBuyButton::get_unit_cost() const
{
	return unit_cost_;
}

void UnitBuyButton::write_to_packet(sf::Packet& packet) const
{
	packet << remaining_time_;
}

void UnitBuyButton::update_from_packet(sf::Packet& packet)
{
	packet >> remaining_time_;
	process(0);
}

void UnitBuyButton::process(const int elapsed_time)
{
	remaining_time_ = std::max(remaining_time_ - elapsed_time, 0);

	const std::string temp_str = "x" + std::to_string(static_cast<int>(ceil(static_cast<float>(remaining_time_) / static_cast<float>(wait_time_))));
	count_text_.setString(temp_str);

	const int remaining_time_for_current_unit = remaining_time_ % wait_time_;
	time_bar_.setSize({ bar_size_.x * static_cast<float>(remaining_time_for_current_unit) / static_cast<float>(wait_time_), bar_size_.y });
}


UserInterface::UserInterface()
{
	gold_sprite_.setTexture(texture_holder.get_texture(gold));
	gold_sprite_.setPosition({ 20, 20 });
	gold_sprite_.setScale({ 0.1f, 0.1f });

	money_count_text_.setFont(text_font);
	money_count_text_.setPosition(20, 70);

	stick_man_.setTexture(texture_holder.get_texture(stick_man));
	stick_man_.setScale({ 0.25, 0.25 });
	stick_man_.setPosition({ 35, 120 });

	army_count_text_.setFont(text_font);
	army_count_text_.setPosition({ 25, 170 });

	total_damage_text_.setFont(text_font);
	total_damage_text_.setPosition({ 20, 250 });
	total_kills_text_.setFont(text_font);
	total_kills_text_.setPosition({ 20, 300 });
	stun_time_left_text_.setFont(text_font);
	stun_time_left_text_.setPosition({ 20, 350 });

	unit_buy_buttons_.push_back(std::make_unique<UnitBuyButton>(Miner::cost, Miner::wait_time, sf::Vector2f{ 130, 20 }, sf::Vector2f{ 0.15f, 0.15f },
		miner_buy_button));

	unit_buy_buttons_.push_back(std::make_unique<UnitBuyButton>(Swordsman::cost, Swordsman::wait_time, sf::Vector2f{ 230, 20 }, sf::Vector2f{ 0.15f, 0.15f },
		swordsman_buy_button));

	unit_buy_buttons_.push_back(std::make_unique<UnitBuyButton>(Magikill::cost, Magikill::wait_time, sf::Vector2f{ 330, 20 }, sf::Vector2f{ 0.15f, 0.15f }, 
		magikill_buy_button));

	unit_buy_buttons_.push_back(std::make_unique<UnitBuyButton>(Spearton::cost, Spearton::wait_time, sf::Vector2f{ 430, 20 }, sf::Vector2f{ 0.15f, 0.15f },
		spearton_buy_button));

	unit_buy_buttons_.push_back(std::make_unique<UnitBuyButton>(Archer::cost, Archer::wait_time, sf::Vector2f{ 530, 20 }, sf::Vector2f{ 0.15f, 0.15f },
		archer_buy_button));

	defend_button_.reset(new Button({ 900.0f, 20.0f }, { 0.15f, 0.15f }, defend_button));
	defend_button_->highlight(true);
	in_attack_button_.reset(new Button({ 1000.0f, 20.0f }, { 0.15f, 0.15f }, in_attack_button));
	escape_button_.reset(new Button({ 800.f, 20.f }, { 0.15f, 0.15f }, escape_button));
}

void UserInterface::update(const Army::ArmyReturnType values, const int stun_time, const int army_count, const std::optional<int> unit_queue_id, const sf::Time delta_time)
{
	// update army count text
	const std::string str = std::to_string(army_count) + "/" + std::to_string(Army::size_per_one_player);
	army_count_text_.setString(str);

	if(in_attack_button_->is_pressed())
	{
		in_attack_button_->highlight(true);
		defend_button_->highlight(false);
		escape_button_->highlight(false);
	}
	else if(defend_button_->is_pressed())
	{
		in_attack_button_->highlight(false);
		defend_button_->highlight(true);
		escape_button_->highlight(false);
	}
	else if(escape_button_->is_pressed())
	{
		in_attack_button_->highlight(false);
		defend_button_->highlight(false);
		escape_button_->highlight(true);
	}

	// process units queue and units spawn
	if (unit_queue_id)
		unit_buy_buttons_[*unit_queue_id]->process(delta_time.asMilliseconds());

	money_count_text_.setString(std::to_string(values.gold_count));
	total_damage_text_.setString("damage: " + std::to_string(static_cast<int>(values.damage)));
	total_kills_text_.setString("kills: " + std::to_string(values.kills));

	if (stun_time > 0)
	{
		stun_time_left_text_.setString("stun left: " + std::to_string(stun_time / 1000) + "." + std::to_string(stun_time % 1000) + "s");
	}
	else
		stun_time_left_text_.setString("");
}

void UserInterface::draw(DrawQueue& queue) const
{
	queue.emplace(interface_layer_0, &gold_sprite_);
	queue.emplace(interface_layer_0, &money_count_text_);
	queue.emplace(interface_layer_0, &total_damage_text_);
	queue.emplace(interface_layer_0, &total_kills_text_);
	queue.emplace(interface_layer_0, &stun_time_left_text_);

	for (const auto& unit_button : unit_buy_buttons_)
		unit_button->draw(queue);

	in_attack_button_->draw(queue);
	defend_button_->draw(queue);
	escape_button_->draw(queue);

	queue.emplace(interface_layer_0, &stick_man_);
	queue.emplace(interface_layer_0, &army_count_text_);
}

std::vector<std::unique_ptr<UnitBuyButton>>& UserInterface::get_unit_buy_buttons()
{
	return unit_buy_buttons_;
}

std::unique_ptr<Button>& UserInterface::get_in_attack_button()
{
	return in_attack_button_;
}

std::unique_ptr<Button>& UserInterface::get_defend_button()
{
	return defend_button_;
}

std::unique_ptr<Button>& UserInterface::get_escape_button()
{
	return escape_button_;
}

void UserInterface::write_to_packet(sf::Packet& packet) const
{
	for (const auto& unit_buy_button : unit_buy_buttons_)
		unit_buy_button->write_to_packet(packet);
}

void UserInterface::update_from_packet(sf::Packet& packet, const Army::ArmyTarget target) const
{
	for (const auto& unit_buy_button : unit_buy_buttons_)
		unit_buy_button->update_from_packet(packet);

	if (target == Army::attack)
		in_attack_button_->press_left();
	else if (target == Army::defend)
		defend_button_->press_left();
	else if (target == Army::escape)
		escape_button_->press_left();
}
