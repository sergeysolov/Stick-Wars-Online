#pragma once
#include <unordered_map>
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

	star,

	goldmine,

	my_statue,
	enemy_statue,

	my_miner,
	enemy_miner,
	my_swordsman,
	enemy_swordsman
};


class TextureHolder
{
	void append(ID id, const char* filepath);
public:
	sf::Texture& get_texture(ID id);

	void load_textures();
private:
	std::unordered_map<ID, sf::Texture> textures_;
};

