#pragma once
#include <SFML/Audio.hpp>
#include <unordered_map>

enum sound_buffer_id
{
	sward_hit,
	sward_damage,
	sward_kill
};

class SoundBuffersHolder
{
	std::unordered_map<sound_buffer_id, sf::SoundBuffer> sound_buffers_;
	void append(sound_buffer_id id, const char* filepath);
public:
	SoundBuffersHolder();

	sf::SoundBuffer& get_sound_buffer(sound_buffer_id id);
};

inline SoundBuffersHolder sound_buffers_holder;