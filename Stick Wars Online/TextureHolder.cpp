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

	append(forest_background, "Images/backgrounds/large_forest.png");
	append(winter_background, "Images/backgrounds/large_winter.png");

	append(my_miner, "Images/units/miner/miner.png");
	append(my_miner_blue, "Images/units/miner/miner_blue.png");
	append(my_miner_orange, "Images/units/miner/miner_orange.png");
	append(my_miner_purple, "Images/units/miner/miner_purple.png");
	append(enemy_miner, "Images/units/miner/miner_enemy.png");

	append(my_swordsman, "Images/units/swordsman/swordsman.png");
	append(my_swordsman_blue, "Images/units/swordsman/swordsman_blue.png");
	append(my_swordsman_orange, "Images/units/swordsman/swordsman_orange.png");
	append(my_swordsman_purple, "Images/units/swordsman/swordsman_purple.png");
	append(enemy_swordsman, "Images/units/swordsman/swordsman_enemy.png");


	append(my_magikill, "Images/units/magikill/magikill.png");
	append(my_magikill_blue, "Images/units/magikill/magikill_blue.png");
	append(my_magikill_orange, "Images/units/magikill/magikill_orange.png");
	append(my_magikill_purple, "Images/units/magikill/magikill_purple.png");
	append(enemy_magikill, "Images/units/magikill/magikill_enemy.png");

	append(my_spearton, "Images/units/spearton/spearton.png");
	append(my_spearton_blue, "Images/units/spearton/spearton_blue.png");
	append(my_spearton_orange, "Images/units/spearton/spearton_orange.png");
	append(my_spearton_purple, "Images/units/spearton/spearton_purple.png");
	append(enemy_spearton, "Images/units/spearton/spearton_enemy.png");

	append(my_archer, "Images/units/archer/archer.png");
	append(my_archer_blue, "Images/units/archer/archer_blue.png");
	append(my_archer_orange, "Images/units/archer/archer_orange.png");
	append(my_archer_purple, "Images/units/archer/archer_purple.png");
	append(enemy_archer, "Images/units/archer/archer_enemy.png");

	append(arrow, "Images/objects/arrow.png");
	append(aim, "Images/objects/aim.png");

	append(gold, "Images/attributes/gold.png");
	append(miner_buy_button, "Images/attributes/miner_buy_button.png");
	append(stick_man, "Images/attributes/stick_man.png");
	append(swordsman_buy_button, "Images/attributes/swardsman_buy_button.png");
	append(magikill_buy_button, "Images/attributes/magikill_buy_button.png");
	append(spearton_buy_button, "Images/attributes/spearton_buy_button.png");
	append(archer_buy_button, "Images/attributes/archer_buy_button.png");
	append(in_attack_button, "Images/attributes/in_attack_button.png");
	append(defend_button, "Images/attributes/defend_button.png");
	append(escape_button, "Images/attributes/escape_button.png");
	append(pause_button, "Images/attributes/pause_button.png");

	append(star, "Images/attributes/star.png");
	append(stun_stars, "Images/attributes/stun_stars.png");

	append(goldmine, "Images/objects/goldmine.png");
	append(barbed_wire, "Images/objects/barbed_wire.png");

	append(my_statue, "Images/objects/statue.png");
	append(enemy_statue, "Images/objects/statue_enemy.png");

	append(explosion_effect, "Images/effects/explosion.png");
}
