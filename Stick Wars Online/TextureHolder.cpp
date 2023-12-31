#include "TextureHolder.h"

void TextureHolder::append(const texture_ID id, const char* filepath)
{
	sf::Texture temp;
	temp.loadFromFile(filepath);
	textures_[id] = temp;
}

sf::Texture& TextureHolder::get_texture(const texture_ID id)
{
	return textures_[id];
}

TextureHolder::TextureHolder()
{
	load_textures();
}

void TextureHolder::load_textures()
{
	append(intro, "Images/backgrounds/intro.png");

	append(forest_background, "Images/backgrounds/forest.png");
	append(large_forest_background, "Images/backgrounds/large_forest.png");

	append(my_miner, "Images/units/miner.png");
	append(my_miner_blue, "Images/units/miner_blue.png");
	append(my_miner_orange, "Images/units/miner_orange.png");
	append(enemy_miner, "Images/units/miner_enemy.png");

	append(my_swordsman, "Images/units/swordsman.png");
	append(my_swordsman_blue, "Images/units/swordsman_blue.png");
	append(my_swordsman_orange, "Images/units/swordsman_orange.png");
	append(enemy_swordsman, "Images/units/swordsman_enemy.png");

	append(my_magikill, "Images/units/magikill.png");
	append(my_magikill_blue, "Images/units/magikill_blue.png");
	append(my_magikill_orange, "Images/units/magikill_orange.png");
	append(enemy_magikill, "Images/units/magikill_enemy.png");

	append(gold, "Images/attributes/gold.png");
	append(miner_buy_button, "Images/attributes/miner_buy_button.png");
	append(stick_man, "Images/attributes/stick_man.png");
	append(swordsman_buy_button, "Images/attributes/swardsman_buy_button.png");
	append(magikill_buy_button, "Images/attributes/magikill_buy_button.png");
	append(in_attack_button, "Images/attributes/in_attack_button.png");
	append(defend_button, "Images/attributes/defend_button.png");
	append(pause_button, "Images/attributes/pause_button.png");

	append(star, "Images/attributes/star.png");
	append(stun_stars, "Images/attributes/stun_stars.png");

	append(goldmine, "Images/objects/goldmine.png");

	append(my_statue, "Images/objects/statue.png");
	append(enemy_statue, "Images/objects/statue_enemy.png");

	append(explosion_effect, "Images/effects/explosion.png");
}
