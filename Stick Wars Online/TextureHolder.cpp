#include "TextureHolder.h"

void TextureHolder::append(ID id, const char* filepath)
{
	sf::Texture temp;
	temp.loadFromFile(filepath);
	_textures.insert(std::make_pair(id, temp));
}

sf::Texture& TextureHolder::getTexture(ID id)
{
	auto result = _textures.find(id);
	return result->second;
}
