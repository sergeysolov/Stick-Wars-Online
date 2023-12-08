#include "TextureHolder.h"

void TextureHolder::append(ID id, const char* filepath)
{
	sf::Texture temp;
	temp.loadFromFile(filepath);
	textures_[id] = temp;
}

sf::Texture& TextureHolder::getTexture(const ID id)
{
	return textures_[id];
}
