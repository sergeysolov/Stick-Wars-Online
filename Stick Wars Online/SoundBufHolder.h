#pragma once
#include <SFML/Audio.hpp>
#include <unordered_map>

enum sound_buffer_id
{
	miner_hit,

	sward_hit,
	sward_damage,
	sward_kill,

	explosion_sound,

	in_attack_music,
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


class SoundManager
{
	struct SoundParams
	{
		int volume;
		int count;
	};

	std::unordered_map<sound_buffer_id, std::pair<int, std::vector<sf::Sound>>> sounds_;
public:
	SoundManager();
	void play_sound(sound_buffer_id sound_id);
};


inline SoundManager sound_manager;