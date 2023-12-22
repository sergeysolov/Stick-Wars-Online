#pragma once
#include <SFML/Network.hpp>
#include <vector>
#include <thread>
#include <iostream>
#include <list>
#include <mutex>
#include <optional>
#include <queue>

#include "Input.h"

class Connection
{
	std::unique_ptr<sf::TcpSocket> socket_;
	int id_;
	std::string name_;

	std::mutex input_mtx_;
	bool send_input_active_ = false;
	bool receive_input_active_ = false;
	std::optional<Input> input_;

	std::mutex update_mtx_;
	bool send_updates_active_ = false;
	bool receive_updates_active_ = false;
	std::shared_ptr<sf::Packet> update_;
public:
	Connection(std::unique_ptr<sf::TcpSocket>&& socket, int id, std::string name);

	[[nodiscard]] sf::TcpSocket& get_socket() const;
	[[nodiscard]] int get_id() const;
	[[nodiscard]] std::string get_name() const;

	void put_input(const Input& input);
	std::optional<Input> get_input();

	void put_update(const std::shared_ptr<sf::Packet>& update);
	std::shared_ptr<sf::Packet> get_update();

	void start_send_input();
	void stop_send_input();

	void start_receive_input();
	void stop_receive_input();

	void start_send_updates();
	void stop_send_updates();

	void start_receive_updates();
	void stop_receive_updates();

	constexpr static int port = 38721;
};

class ServerConnectionHandler
{
	sf::TcpListener listener_;
	std::list<Connection> clients_;
	std::string player_name_;

	std::mutex clients_mtx_;
	bool listen_ = false;
public:
	void listen_for_client_connection();
	void stop_listen();
	~ServerConnectionHandler();
	
	std::list<Connection>& get_connections();
	void read_player_name();
	[[nodiscard]] std::string get_player_name() const;

	void start_receive_input();
	void stop_receive_input();

	void start_send_updates();
	void stop_send_updates();

	std::vector<std::optional<Input>> get_clients_input();
	void put_update_to_clients(const sf::Packet& update_packet);
};

inline std::unique_ptr<ServerConnectionHandler> server_handler;


class ClientConnectionHandler
{
	std::unique_ptr<Connection> server_;
	int id_ = -1;

public:
	void connect();
	Connection& get_server() const;
	[[nodiscard]] int get_id() const;

	void start_send_input() const;
	void stop_send_input() const;

	void start_receive_updates() const;
	void stop_receive_updates() const;

	void put_input_to_server(const Input& input) const;
	std::shared_ptr<sf::Packet> get_update_from_server() const;
};

inline std::unique_ptr<ClientConnectionHandler> client_handler;