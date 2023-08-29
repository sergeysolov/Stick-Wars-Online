#pragma once
#include <map>
#include <SFML/Graphics.hpp>

enum ID
{
	forest_background,
	large_forest_background,

	gold,
	miner_buy_button,
	swordsman_buy_button,
	stick_man,
	in_attack_button,
	defend_button,

	goldmine,

	miner,
	miner_enemy,
	swordsman,
	swordsman_enemy
};


class TextureHolder
{
public:
	void append(ID id, const char* filepath);
	
	sf::Texture& getTexture(ID id);
	
private:
	std::map<ID, sf::Texture> _textures;
};

