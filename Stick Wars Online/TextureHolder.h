#pragma once
#include <unordered_map>
#include <SFML/Graphics.hpp>

enum texture_ID
{
	intro,

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
	void append(texture_ID id, const char* filepath);
	void load_textures();
public:
	sf::Texture& get_texture(texture_ID id);
	TextureHolder();
	
private:
	std::unordered_map<texture_ID, sf::Texture> textures_;
};

inline TextureHolder texture_holder;