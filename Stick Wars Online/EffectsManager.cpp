#include "EffectsManager.h"

#include <functional>

int AnimationParams::get_total_time() const
{
	return total_frames * time_frame;
}

Effect::Effect(const int time, const sf::Vector2f position) : position_(position), time_(time)
{	}


bool Effect::update(const int delta_time)
{
	time_ = std::max(time_ - delta_time, 0);
	return time_ == 0;
}

void Effect::write_to_packet(sf::Packet& packet) const
{
	packet << position_.x << position_.y << time_;
}

void Effect::update_from_packet(sf::Packet& packet)
{
	packet >> position_.x >> position_.y >> time_;
}

SpriteEffect::SpriteEffect(const texture_ID id, const int time, const sf::Vector2f position) : Effect(time, position)
{
	sprite_.setTexture(texture_holder.get_texture(id));
}

void SpriteEffect::draw(DrawQueue& draw_queue)
{
	draw_queue.emplace(attributes_layer_0, &sprite_);
}

bool SpriteEffect::update(const int delta_time)
{
	const bool to_delete = Effect::update(delta_time);

	const auto animation_params = get_animation_params();
	const int current_frame = (animation_params.get_total_time() - time_) / animation_params.time_frame;
	//const int current_frame = static_cast<int>(static_cast<float>(animation_params.get_total_time() - time_) / static_cast<float>(animation_params.get_total_time()) * static_cast<float>(animation_params.total_frames));

	int x_shift = animation_params.init_position.x + animation_params.frame_width * (current_frame % animation_params.frames_in_one_row);
	int y_shift = animation_params.init_position.y + animation_params.frame_height * (current_frame / animation_params.frames_in_one_row);

	sprite_.setTextureRect({ x_shift, y_shift, animation_params.frame_width, animation_params.frame_height });

	return to_delete;
}

void SpriteEffect::set_screen_place(const float camera_position)
{
	sprite_.setPosition({ position_.x - camera_position + get_offset().x, position_.y + get_offset().y });
}

ExplosionEffect::ExplosionEffect(const sf::Vector2f position, const int direction) : SpriteEffect(explosion_effect, animation_params.get_total_time(), position), direction_(direction)
{
	sprite_.setScale(animation_params.scale);
}

const AnimationParams& ExplosionEffect::get_animation_params() const
{
	return animation_params;
}

sf::Vector2f ExplosionEffect::get_offset() const
{
	if (direction_ > 0)
		return { offset.x * static_cast<float>(direction_), offset.y };
	return { offset.x * static_cast<float>(direction_) - static_cast<float>(2 * animation_params.frame_width), offset.y };
}

int ExplosionEffect::get_id() const
{
	return id;
}

void ExplosionEffect::write_to_packet(sf::Packet& packet) const
{
	packet << id << direction_;
	SpriteEffect::write_to_packet(packet);
}

void ExplosionEffect::update_from_packet(sf::Packet& packet)
{
	packet >> direction_;
	SpriteEffect::update_from_packet(packet);
	update(0);
}

TextEffect::TextEffect(const std::string& text, const int time, const sf::Vector2f position) : Effect(time, position)
{
	text_.setFont(text_font);
	text_.setString(text);
}

void TextEffect::draw(DrawQueue& draw_queue)
{
	draw_queue.emplace(attributes_layer_0, &text_);
}

void TextEffect::set_screen_place(const float camera_position)
{
	text_.setPosition({ position_.x - camera_position + get_offset().x, position_.y + get_offset().y });
}


DropDamageEffect::DropDamageEffect(const sf::Vector2f position, const int damage) : TextEffect((damage > 0 ? "-" : "+") + std::to_string(abs(damage)), total_time, position), damage_(damage)
{
	if (damage > 0)
		text_.setFillColor(sf::Color::Red);
	else
		text_.setFillColor(sf::Color::Green);
}

sf::Vector2f DropDamageEffect::get_offset() const
{
	const float y_offset = -static_cast<float>(total_time - time_) / static_cast<float>(total_time) * y_range;
	return { 0, y_offset };
}

void DropDamageEffect::write_to_packet(sf::Packet& packet) const
{
	packet << id << damage_;
	TextEffect::write_to_packet(packet);
}

void DropDamageEffect::update_from_packet(sf::Packet& packet)
{
	packet >> damage_;

	text_.setString((damage_ > 0 ? "-" : "+") + std::to_string(abs(damage_)));

	if (damage_ > 0)
		text_.setFillColor(sf::Color::Red);
	else
		text_.setFillColor(sf::Color::Green);

	TextEffect::update_from_packet(packet);
}

int DropDamageEffect::get_id() const
{
	return id;
}

Effect* EffectsManager::create_effect(const int effect_id)
{
	static std::unordered_map<int, std::function<Effect* ()>> factory =
	{
		{ExplosionEffect::id, [] {return new ExplosionEffect({0.f, 0.f}, 1); }},
		{DropDamageEffect::id, [] {return new DropDamageEffect({0.f, 0.f}, 100); }}
	};
	return factory[effect_id]();
}

void EffectsManager::add_effect(std::unique_ptr<Effect>&& effect)
{
	effects_.push_back(std::move(effect));
}

void EffectsManager::process(const int delta_time)
{
	for (auto it = effects_.begin(); it != effects_.end(); ++it)
	{
		if(it->get()->update(delta_time))
		{
			std::swap(*it, effects_.back());
			effects_.pop_back();
			break;
		}
	}
}

void EffectsManager::draw(DrawQueue& draw_queue) const
{
	for (const auto& effect : effects_)
	{
		effect->draw(draw_queue);
	}
}

void EffectsManager::set_screen_place(const float camera_position) const
{
	for (const auto& effect : effects_)
	{
		effect->set_screen_place(camera_position);
	}
}

void EffectsManager::write_to_packet(sf::Packet& packet) const
{
	packet << effects_.size();
	for (const auto& effect : effects_)
		effect->write_to_packet(packet);
}

void EffectsManager::update_from_packet(sf::Packet& packet)
{
	size_t effects_size;
	packet >> effects_size;

	for (int i = 0; i < std::min(effects_size, effects_.size()); i++)
	{
		int effect_id; packet >> effect_id;
		if (effect_id != effects_[i]->get_id())
			effects_[i].reset(create_effect(effect_id));
		effects_[i]->update_from_packet(packet);
	}

	if (effects_size < effects_.size())
		effects_.resize(effects_size);
	else
	{
		while (effects_.size() < effects_size)
		{
			int effect_id; packet >> effect_id;
			effects_.emplace_back(create_effect(effect_id));
			effects_.back()->update_from_packet(packet);
		}
	}
}


