#include "SoundBufHolder.h"

#include <random>
#include <ranges>

void SoundBuffersHolder::append(const sound_buffer_id id, const char* filepath)
{
	sf::SoundBuffer sound_buffer;
	sound_buffer.loadFromFile(filepath);
	sound_buffers_[id] = sound_buffer;
}

SoundBuffersHolder::SoundBuffersHolder()
{
	append(miner_hit, "Sounds/unit_sounds/miner_hit.mp3");
	append(money_sound, "Sounds/unit_sounds/money_sound.mp3");

	append(statue_damage_sound, "Sounds/unit_sounds/statue_damage.mp3");

	append(sward_hit, "Sounds/unit_sounds/sward_hit.wav");
	append(sward_damage, "Sounds/unit_sounds/sward_damage.wav");
	append(sward_kill, "Sounds/unit_sounds/sward_kill.wav");

	append(spearton_second_attack_sound, "Sounds/unit_sounds/spearton_second_attack_sound.mp3");

	append(explosion_sound, "Sounds/unit_sounds/explosion_sound.mp3");

	append(in_attack_music, "Sounds/music/in_attack_music.wav");

	append(victory_music, "Sounds/music/victory_sound.mp3");
	append(background_music_0, "Sounds/music/background_music_1.mp3");
	append(background_music_1, "Sounds/music/background_music_2.mp3");
	
}

sf::SoundBuffer& SoundBuffersHolder::get_sound_buffer(const sound_buffer_id id)
{
	return sound_buffers_[id];
}


SoundManager::SoundManager(const std::unordered_map<sound_buffer_id, SoundParams>& sounds_params)
{
	for (const auto& [buffer_id, params] : sounds_params)
	{
		std::vector<sf::Sound> array_sounds;
		for (int i = 0; i < params.count; i++)
		{
			array_sounds.emplace_back();
			array_sounds.back().setBuffer(sound_buffers_holder.get_sound_buffer(buffer_id));
			array_sounds.back().setVolume(params.volume);
		}
		sounds_.insert({ buffer_id, { 0, array_sounds } });
	}
}

void SoundManager::play_sound(const sound_buffer_id sound_id)
{
	auto& [idx, sounds] = sounds_[sound_id];

	idx = (idx + 1) % sounds.size();
	if (sounds[idx].getStatus() != sf::Sound::Playing)
		sounds[idx].play();
}

SharedSoundManager::SharedSoundManager(const std::unordered_map<sound_buffer_id, SoundParams>& sounds_params) : SoundManager(sounds_params)
{
}

void SharedSoundManager::play_sound(const sound_buffer_id sound_id)
{
	SoundManager::play_sound(sound_id);
	played_sounds_counts_[sound_id]++;
}

void SharedSoundManager::write_to_packet(sf::Packet& packet)
{
	packet << played_sounds_counts_.size();
	for (const auto [id, count] : played_sounds_counts_)
		packet << id << count;

	played_sounds_counts_.clear();
}

void SharedSoundManager::update_from_packet(sf::Packet& packet)
{
	size_t size; packet >> size;

	for (const auto i : std::views::iota(0u, size))
	{
		int id, play_count; packet >> id >> play_count;
		for (const auto j : std::views::iota(0, play_count))
			play_sound(static_cast<sound_buffer_id> (id));
	}
}

MusicManager::MusicManager(const std::unordered_map<sound_buffer_id, float>& musics_params)
{
	for (const auto [buffer_id, volume] : musics_params)
	{
		auto& music = musics_[buffer_id]; 
		music.setBuffer(sound_buffers_holder.get_sound_buffer(buffer_id));
		music.setVolume(volume);
	}
}

void MusicManager::play_new_background_music()
{
	static std::uniform_int_distribution distribution(0, total_background_musics - 1);
	static std::mt19937 generator(std::random_device{}());

	current_background_music_ = static_cast<sound_buffer_id>(distribution(generator));

	auto& music = musics_[current_background_music_];
	music.play();
	music.setLoop(true);
}

void MusicManager::continue_background_music()
{
	musics_[current_background_music_].play();
}

void MusicManager::pause_background_music()
{
	musics_[current_background_music_].pause();
}

void MusicManager::play_music(const sound_buffer_id music_id)
{
	musics_[music_id].play();
}

void MusicManager::pause_music(const sound_buffer_id music_id)
{
	musics_[music_id].pause();
}

void MusicManager::stop_all()
{
	for (sf::Sound& music : musics_ | std::views::values)
		music.stop();
}

void MusicManager::write_to_packet(sf::Packet& packet) const
{
	packet << current_background_music_;
}

void MusicManager::update_from_packet(sf::Packet& packet)
{
	int current_background_music;
	packet >> current_background_music;
	current_background_music_ = static_cast<sound_buffer_id>(current_background_music);

	auto& music = musics_[current_background_music_];
	music.play();
	music.setLoop(true);
}


PrivateSoundManager::PrivateSoundManager(const std::unordered_map<sound_buffer_id, SoundParams>& sounds_params) : SoundManager(sounds_params)
{
}

void PrivateSoundManager::set_active(const bool is_active)
{
	is_active_ = is_active;
}

void PrivateSoundManager::play_sound(const sound_buffer_id sound_id)
{
	if(is_active_)
		SoundManager::play_sound(sound_id);

}
