#pragma once
#include "BaseState.h"
class SpawnUnitQueue;
class Army;
class StateManager;

class IPlayState : public BaseState
{
public:
	virtual float get_camera_position() const = 0;
	virtual int& get_money_count() = 0;
	[[nodiscard]] virtual SpawnUnitQueue& get_SpawnQueue() const = 0;
	virtual Army& get_Army() = 0;
	[[nodiscard]] virtual StateManager& get_StateManager() const = 0;
};

