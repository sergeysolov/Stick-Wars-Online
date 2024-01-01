#include "EffectsManager.h"

#include <functional>
#include <ranges>

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
	packet << position_.x << position_.y;
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

sf::Vector2f TextEffect::get_offset() const
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

DropMoneyEffect::DropMoneyEffect(const sf::Vector2f position, const int money) : TextEffect("+" + std::to_string(money), total_time, position), money_(money)
{
	text_.setFillColor(sf::Color::Yellow);
}

int DropMoneyEffect::get_id() const
{
	return id;
}

Effect* EffectsManager::create_effect(const int effect_id, const sf::Vector2f position, const int param)
{
	static std::unordered_map<int, std::function<Effect*(sf::Vector2f, int)>> factory =
	{
		{ExplosionEffect::id, [] (const sf::Vector2f pos, const int p)
		{
			return new ExplosionEffect(pos, p);
		}},
		{DropDamageEffect::id, [](const sf::Vector2f pos, const int p)
		{
			return new DropDamageEffect(pos, p);
		}},
		{DropMoneyEffect::id, [](const sf::Vector2f pos, const int p)
		{
			return new DropMoneyEffect(pos, p);
		}}
	};
	return factory[effect_id](position, param);
}

void EffectsManager::add_effect(std::unique_ptr<Effect>&& effect)
{
	effects_.push_back(std::move(effect));
}

void EffectsManager::process(const int delta_time)
{
	for (size_t i = 0; i < effects_.size(); )
	{
		if (effects_[i]->update(delta_time))
		{
			std::swap(effects_[i], effects_.back());
			effects_.pop_back();
		}
		else
			i++;
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


void SharedEffectManager::add_effect(std::unique_ptr<Effect>&& effect)
{
	added_effects_.push_back(std::move(effect));
}

void SharedEffectManager::process(const int delta_time)
{
	std::ranges::move(added_effects_, std::back_inserter(effects_));
	added_effects_.clear();
	EffectsManager::process(delta_time);
}

void SharedEffectManager::write_to_packet(sf::Packet& packet) const
{
	packet << added_effects_.size();
	for (const auto& effect : added_effects_)
		effect->write_to_packet(packet);
}

void SharedEffectManager::update_from_packet(sf::Packet& packet)
{
	size_t new_effects_count;
	packet >> new_effects_count;

	for (size_t i = 0; i < new_effects_count; i++)
	{
		int effect_id, param; sf::Vector2f position;
		packet >> effect_id >> param >> position.x >> position.y;
		added_effects_.emplace_back(create_effect(effect_id, position, param));
	}
}

void PrivateEffectManager::set_active(const bool is_active)
{
	is_active_ = is_active;
}

void PrivateEffectManager::add_effect(std::unique_ptr<Effect>&& effect)
{
	if(is_active_)
		EffectsManager::add_effect(std::move(effect));
}


