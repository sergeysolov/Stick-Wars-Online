#pragma once
#include <unordered_map>
#include <SFML/Graphics.hpp>

enum texture_ID
{
	none,

	intro,

	forest_background,
	large_forest_background,

	gold,
	miner_buy_button,
	swordsman_buy_button,
	magikill_buy_button,
	spearton_buy_button,
	archer_buy_button,
	stick_man,
	in_attack_button,
	defend_button,
	escape_button,
	pause_button,

	star,
	stun_stars,

	goldmine,
	barbed_wire,

	my_statue,
	enemy_statue,

	enemy_miner,
	my_miner,
	my_miner_blue,
	my_miner_orange,
	
	enemy_swordsman,
	my_swordsman,
	my_swordsman_blue,
	my_swordsman_orange,

	enemy_magikill,
	my_magikill,
	my_magikill_blue,
	my_magikill_orange,

	enemy_spearton,
	my_spearton,
	my_spearton_blue,
	my_spearton_orange,

	enemy_archer,
	my_archer,
	my_archer_blue,
	my_archer_orange,

	arrow,
	aim,

	explosion_effect,
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

inline sf::Font text_font;