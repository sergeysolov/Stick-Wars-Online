#include "Input.h"


void Input::process_events(sf::RenderWindow& window)
{
	auto key_manage = [&](const sf::Event event, const bool is_pressed)
		{
			if (event.key.code == sf::Keyboard::D)
				d = is_pressed;
			if (event.key.code == sf::Keyboard::A)
				a = is_pressed;
			if (event.key.code == sf::Keyboard::W)
				w = is_pressed;
			if (event.key.code == sf::Keyboard::S)
				s = is_pressed;
			if (event.key.code == sf::Keyboard::K)
				k = is_pressed;
			if (event.key.code == sf::Keyboard::Left)
				left_arrow = is_pressed;
			if (event.key.code == sf::Keyboard::Right)
				right_arrow = is_pressed;
			if (event.key.code == sf::Keyboard::Space)
				space = is_pressed;
			if (event.key.code == sf::Keyboard::LShift)
				shift = is_pressed;
			if (event.key.code == sf::Keyboard::Escape)
				escape = is_pressed;
		};

	sf::Event event{};

	mouse_left = false;
	while (window.pollEvent(event))
	{
		switch (event.type)
		{
		case sf::Event::KeyPressed:
			key_manage(event, true);
			break;
		case sf::Event::KeyReleased:
			key_manage(event, false);
			break;
		case sf::Event::MouseButtonReleased:
			if (event.mouseButton.button == sf::Mouse::Left)
			{
				mouse_left = true;
				mouse_position = sf::Mouse::getPosition(window);
			}
			break;
		case sf::Event::Closed:
			window.close();
		default:
			break;
		}
	}
}
