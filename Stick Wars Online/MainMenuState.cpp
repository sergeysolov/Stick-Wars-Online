#include "MainMenuState.h"

MainMenuState::MainMenuState(StateManager& state_manager) : state_manager_(state_manager)
{
	background_.setSize(static_cast<sf::Vector2f>(sf::Vector2u{ sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height }));
	background_.setTexture(&texture_holder.get_texture(intro));

	play_button_ = std::make_unique<Button>(sf::Vector2f{ 170, 200 }, sf::Vector2f{ 250, 100 }, sf::Color::Black);
	play_button_->get_text().setFont(text_font);
	play_button_->get_text().setString("Play");
	play_button_->get_text().move({ 90, 30 });

	exit_button_ = std::make_unique<Button>(sf::Vector2f{ 170, 350 }, sf::Vector2f{ 250, 100 }, sf::Color::Black);
	exit_button_->get_text().setFont(text_font);
	exit_button_->get_text().setString("Exit");
	exit_button_->get_text().move({ 90, 30 });

}

void MainMenuState::update(sf::Time delta_time)
{
	if (play_button_->is_pressed())
		state_manager_.switch_state(play);
	if (exit_button_->is_pressed())
		state_manager_.switch_state(to_exit);
}

void MainMenuState::handle_input(Input& input, sf::Time delta_time)
{
	if (play_button_->check_mouse_pressed(input.mouse_position))
		play_button_->press();
	else if (exit_button_->check_mouse_pressed(input.mouse_position))
		exit_button_->press();
}

void MainMenuState::draw(DrawQueue& draw_queue)
{
	draw_queue.emplace(background, &background_);
	play_button_->draw(draw_queue);
	exit_button_->draw(draw_queue);
}
