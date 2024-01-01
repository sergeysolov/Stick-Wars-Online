#pragma once
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <SFML/Network/Packet.hpp>

enum sound_buffer_id
{
	miner_hit,
	money_sound,

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
protected:
	struct SoundParams
	{
		int volume;
		int count;
	};

	std::unordered_map<sound_buffer_id, std::pair<int, std::vector<sf::Sound>>> sounds_;
	
public:
	virtual ~SoundManager() = default;
	virtual void play_sound(sound_buffer_id sound_id);
};


class SharedSoundManager : public SoundManager
{
	std::unordered_map<sound_buffer_id, int> played_sounds_counts_;
public:
	SharedSoundManager();

	void play_sound(sound_buffer_id sound_id) override;

	void write_to_packet(sf::Packet& packet);
	void update_from_packet(sf::Packet& packet);
};


class PrivateSoundManager : public SoundManager
{
	bool is_active_ = true;
public:
	PrivateSoundManager();
	void set_active(bool is_active);
	void play_sound(sound_buffer_id sound_id) override;
};

inline SharedSoundManager shared_sound_manager;
inline PrivateSoundManager private_sound_manager;