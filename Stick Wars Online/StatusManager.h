#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>

enum StatusType
{
	main_menu,
	play,
	pause,
	lose_menu,
	victory_menu
};

class IGameState
{
public:
	virtual void init() = 0;
	virtual void update(sf::Time delta_time) = 0;
	virtual void handle_input(sf::Time delta_time) = 0;
	virtual void draw() = 0;
	virtual void cleanup() = 0;
};

class PlayStatus : public IGameState
{
public:
	void init() override;
	void update(sf::Time delta_time) override;
	void handle_input(sf::Time delta_time) override;
	void draw() override;
	void cleanup() override;
};

class StatusManager
{
	std::unordered_map<StatusType, std::is_function<void()>> status_to_action_;
	std::unique_ptr<IGameState> current_state_;
public:


};

