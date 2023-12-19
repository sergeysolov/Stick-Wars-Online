#pragma once
#include <SFML/Network.hpp>
#include <vector>
#include <thread>
#include <iostream>
#include <mutex>

class Connection
{
	std::unique_ptr<sf::TcpSocket> socket_;
	int id_;
	std::string name_;
public:
	Connection(std::unique_ptr<sf::TcpSocket>&& socket, int id, std::string name);
	constexpr static int port = 38721;
};


class ServerConnectionHandler
{
	sf::TcpListener listener_;
	std::vector<Connection> clients_;
	std::mutex clients_mtx_;
	
public:
	void listen_for_client_connection();
};

inline std::unique_ptr<ServerConnectionHandler> server_handler;


class ClientConnectionHandler
{
	std::unique_ptr<Connection> server_;
	int id_ = -1;
public:
	void connect();
	[[nodiscard]] int get_id() const;
};

inline std::unique_ptr<ClientConnectionHandler> client_handler;