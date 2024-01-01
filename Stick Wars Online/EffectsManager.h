#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network/Packet.hpp>

#include "TextureHolder.h"
#include "DrawQueue.h"

struct AnimationParams
{
	sf::Vector2i init_position;
	int frame_height;
	int frame_width;
	sf::Vector2f scale;
	int total_frames;
	int frames_in_one_row = total_frames;
	int time_frame;

	AnimationParams(const sf::Vector2i init_position, const int frame_height, const int frame_width, const sf::Vector2f scale, const int total_frames, const int time_frame)
		: init_position(init_position), frame_height(frame_height), frame_width(frame_width), scale(scale), total_frames(total_frames), time_frame(time_frame)
	{}

	AnimationParams(const sf::Vector2i init_position, const int frame_height, const int frame_width, const sf::Vector2f scale, const int total_frames, const int time_frame, const int frames_in_one_row)
		: init_position(init_position), frame_height(frame_height), frame_width(frame_width), scale(scale), total_frames(total_frames), frames_in_one_row(frames_in_one_row), time_frame(time_frame)
	{}

	[[nodiscard]] int get_total_time() const;
};

class Effect
{
protected:
	sf::Vector2f position_;
	int time_;
public:
	Effect(int time, sf::Vector2f position);
	virtual ~Effect() = default;

	virtual sf::Vector2f get_offset() const = 0;

	virtual void draw(DrawQueue& draw_queue) = 0;
	virtual bool update(int delta_time);
	virtual void set_screen_place(float camera_position) = 0;

	virtual int get_id() const = 0;

	virtual void write_to_packet(sf::Packet& packet) const;
	virtual void update_from_packet(sf::Packet& packet);
};


class SpriteEffect : public Effect
{
protected:
	sf::Sprite sprite_;
	SpriteEffect(texture_ID id, int time, sf::Vector2f position);

public:
	virtual const AnimationParams& get_animation_params() const = 0;

	void draw(DrawQueue& draw_queue) override;
	bool update(int delta_time) override;
	void set_screen_place(float camera_position) override;
};



class ExplosionEffect : public SpriteEffect
{
	int direction_;

	const inline static sf::Vector2f offset = { 100.f, -120.f }; // 130, -120
	const inline static AnimationParams animation_params = { {0, 0}, 194, 200, {2.f, 2.f}, 15, 45, 5 };
public:
	constexpr static int id = 0;

	ExplosionEffect(sf::Vector2f position, int direction);
	const AnimationParams& get_animation_params() const override;
	sf::Vector2f get_offset() const override;

	int get_id() const override;

	void write_to_packet(sf::Packet& packet) const override;
	void update_from_packet(sf::Packet& packet) override;
};


class TextEffect : public Effect
{
protected:
	sf::Text text_;
	TextEffect(const std::string& text, int time, sf::Vector2f position);

	constexpr static int total_time = 1000;
	const inline static sf::Vector2f offset = { 0, -10 };
	constexpr static float y_range = 200;

	sf::Vector2f get_offset() const override;
public:

	void draw(DrawQueue& draw_queue) override;
	void set_screen_place(float camera_position) override;
};


class DropDamageEffect : public TextEffect
{
	int damage_;
public:
	constexpr static int id = 1;
	DropDamageEffect(sf::Vector2f position, int damage);

	void write_to_packet(sf::Packet& packet) const override;
	void update_from_packet(sf::Packet& packet) override;

	int get_id() const override;
};

class DropMoneyEffect : public TextEffect
{
	int money_;
public:
	constexpr static int id = 2;
	DropMoneyEffect(sf::Vector2f position, int money);

	int get_id() const override;
};


class EffectsManager
{
protected:
	static Effect* create_effect(int effect_id, sf::Vector2f position, int param);

	std::vector<std::unique_ptr<Effect>> effects_;
	
public:
	virtual ~EffectsManager() = default;

	virtual void add_effect(std::unique_ptr<Effect>&& effect);
	virtual void process(int delta_time);
	void draw(DrawQueue& draw_queue) const;
	void set_screen_place(float camera_position) const;

};


class SharedEffectManager : public EffectsManager
{
	std::vector<std::unique_ptr<Effect>> added_effects_;
public:
	void add_effect(std::unique_ptr<Effect>&& effect) override;
	void process(int delta_time) override;

	void write_to_packet(sf::Packet& packet) const;
	void update_from_packet(sf::Packet& packet);
};

class PrivateEffectManager : public EffectsManager
{
	bool is_active_ = true;
public:
	void set_active(bool is_active);
	void add_effect(std::unique_ptr<Effect>&& effect) override;

};

inline SharedEffectManager shared_effects_manager;
inline PrivateEffectManager private_effect_manager;
