#include "SoundBufHolder.h"

void SoundBuffersHolder::append(const sound_buffer_id id, const char* filepath)
{
	sf::SoundBuffer sound_buffer;
	sound_buffer.loadFromFile(filepath);
	sound_buffers_[id] = sound_buffer;
}

SoundBuffersHolder::SoundBuffersHolder()
{
	append(miner_hit, "Sounds/unit_sounds/miner_hit.mp3");

	append(sward_hit, "Sounds/unit_sounds/sward_hit.wav");
	append(sward_damage, "Sounds/unit_sounds/sward_damage.wav");
	append(sward_kill, "Sounds/unit_sounds/sward_kill.wav");

	append(explosion_sound, "Sounds/unit_sounds/explosion_sound.mp3");

	append(in_attack_music, "Sounds/music/in_attack_music.wav");
	
}

sf::SoundBuffer& SoundBuffersHolder::get_sound_buffer(const sound_buffer_id id)
{
	return sound_buffers_[id];
}


SoundManager::SoundManager()
{
	const std::unordered_map<sound_buffer_id, SoundParams> sounds_params =
	{
		{sward_kill, {10, 3}},
		{sward_damage, {10, 8}},
		{sward_hit, {10, 7}},
		{explosion_sound, {100, 4}},
		{miner_hit, {1, 5}}
	};

	for (const auto& sound_params : sounds_params)
	{
		std::vector<sf::Sound> array_sounds;
		for (int i = 0; i < sound_params.second.count; i++)
		{
			array_sounds.emplace_back();
			array_sounds.back().setBuffer(sound_buffers_holder.get_sound_buffer(sound_params.first));
			array_sounds.back().setVolume(sound_params.second.volume);
		}
		sounds_.insert({ sound_params.first, { 0, array_sounds } });
	}

}

void SoundManager::play_sound(const sound_buffer_id sound_id)
{
	auto& [idx, sounds] = sounds_[sound_id];

	idx = (idx + 1) % sounds.size();
	if (sounds[idx].getStatus() != sf::Sound::Playing)
		sounds[idx].play();
}
