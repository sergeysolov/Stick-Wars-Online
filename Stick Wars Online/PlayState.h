#pragma once
#include "BaseState.h"
#include "MapObject.h"
#include "Units.h"
#include "UserInterface.h"
#include "StateManager.h"
#include "Player.h"

constexpr float max_camera_position = map_frame_width * 2 + 150; // map_frame_width * 2
constexpr float min_camera_position = 0; // 0
constexpr float start_camera_position = 0;

static int enemy_behaviour = 0;

class StateManager;
class Player;
class Army;
class SpawnUnitQueue;

class PlayState : public BaseState
{
	StateManager& state_manager_;

	sf::Sprite background_sprite_;
	BarbedWire my_barbed_wire_;
	BarbedWire enemy_barbed_wire_;

	std::unique_ptr<Button> pause_button_;
	sf::Text camera_position_text_;
	sf::Text enemy_army_count_text_;

	float camera_position_ = start_camera_position;

	std::vector<Player> players_; // shared

	std::shared_ptr<Statue> my_statue_; //shared
	std::shared_ptr<Statue> enemy_statue_; //shared
	std::unique_ptr<Army> enemy_army_;// shared
	std::unique_ptr<SpawnUnitQueue> enemy_spawn_queue_;
	std::vector<std::shared_ptr<GoldMine>> gold_mines_;//shared 

	void move_camera(float step);
	void set_objects_screen_place();

public:
	explicit PlayState(StateManager& state_manager);
	~PlayState() override;

	void update(sf::Time delta_time) override;
	void handle_input(Input& input, sf::Time delta_time) override;
	void draw(DrawQueue& draw_queue) override;

protected:
	void write_to_packet() const;
	void update_from_packet();

};

