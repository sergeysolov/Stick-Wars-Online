#pragma once
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <SFML/Network/Packet.hpp>

enum sound_buffer_id
{
	background_music_0,
	background_music_1,
	victory_music,

	miner_hit,
	money_sound,

	sward_hit,
	sward_damage,
	sward_kill,

	spearton_second_attack_sound,

	statue_damage_sound,

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
	std::unordered_map<sound_buffer_id, std::pair<int, std::vector<sf::Sound>>> sounds_;
public:
	struct SoundParams
	{
		float volume;
		int count;
	};
	using sound_init_type = std::unordered_map<sound_buffer_id, SoundParams>;

	SoundManager(const std::unordered_map<sound_buffer_id, SoundParams>& sounds_params);
	virtual ~SoundManager() = default;
	virtual void play_sound(sound_buffer_id sound_id);
};


class SharedSoundManager : public SoundManager
{
	std::unordered_map<sound_buffer_id, int> played_sounds_counts_;
public:
	SharedSoundManager(const std::unordered_map<sound_buffer_id, SoundParams>& sounds_params);

	void play_sound(sound_buffer_id sound_id) override;

	void write_to_packet(sf::Packet& packet);
	void update_from_packet(sf::Packet& packet);
};


class MusicManager
{
	std::unordered_map<sound_buffer_id, sf::Sound> musics_;

	sound_buffer_id current_background_music_ = background_music_0;
	static constexpr int total_background_musics = 2;
public:
	using music_init_type = std::unordered_map<sound_buffer_id, float>;

	MusicManager(const std::unordered_map<sound_buffer_id, float>& musics_params);

	void play_new_background_music();
	void continue_background_music();
	void pause_background_music();

	void play_music(sound_buffer_id music_id);
	void pause_music(sound_buffer_id music_id);
	void stop_all();

	void write_to_packet(sf::Packet& packet) const;
	void update_from_packet(sf::Packet& packet);
};


class PrivateSoundManager : public SoundManager
{
	bool is_active_ = true;
public:
	PrivateSoundManager(const std::unordered_map<sound_buffer_id, SoundParams>& sounds_params);
	void set_active(bool is_active);
	void play_sound(sound_buffer_id sound_id) override;
};

inline SharedSoundManager shared_sound_manager = SoundManager::sound_init_type {
	{sward_kill, {10, 3}},
	{sward_damage, {10, 8}},
	{sward_hit, {10, 7}},
	{explosion_sound, {100, 4}},
	{spearton_second_attack_sound, {100, 1}},
	{miner_hit, {1, 5}},
	{statue_damage_sound, {40, 6}}
};

inline PrivateSoundManager private_sound_manager = SoundManager::sound_init_type{
	{money_sound,	{10, 3}}
};

inline MusicManager music_manager = MusicManager::music_init_type{
	{victory_music, 50.f},
    {background_music_0, 50.f},
	{background_music_1, 50.f},
};