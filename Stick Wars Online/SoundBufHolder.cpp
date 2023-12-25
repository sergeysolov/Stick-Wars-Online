#include "SoundBufHolder.h"

void SoundBuffersHolder::append(const sound_buffer_id id, const char* filepath)
{
	sf::SoundBuffer sound_buffer;
	sound_buffer.loadFromFile(filepath);
	sound_buffers_[id] = sound_buffer;
}

SoundBuffersHolder::SoundBuffersHolder()
{
	append(sward_hit, "Sounds/sward_hits/sward_hit.wav");
	append(sward_damage, "Sounds/sward_hits/sward_damage.wav");
	append(sward_kill, "Sounds/sward_hits/sward_kill.wav");
	append(in_attack_music, "Sounds/music/in_attack_music.wav");
}

sf::SoundBuffer& SoundBuffersHolder::get_sound_buffer(const sound_buffer_id id)
{
	return sound_buffers_[id];
}
