#include "ConnectionHandler.h"

Connection::Connection(std::unique_ptr<sf::TcpSocket>&& socket, const int id, std::string name) :
socket_(std::move(socket)), id_(id), name_(std::move(name))
{	}

sf::TcpSocket& Connection::get_socket() const
{
	return *socket_;
}

void ClientConnectionHandler::connect()
{
	std::thread([&]
		{
			std::cout << "Input server ip address: \n";
			std::string server_address_str;
			std::cin >> server_address_str;

			if (const sf::IpAddress server_address(server_address_str); server_address != sf::IpAddress::None)
			{
				if (auto server = std::make_unique<sf::TcpSocket>(); server->connect(server_address, Connection::port) == sf::TcpSocket::Done)
				{
					std::cout << "Input your name: \n";
					std::string name;
					std::cin >> name;
					sf::Packet name_packet; name_packet << name;
					if (server->send(name_packet) != sf::TcpSocket::Done)
					{
						std::cout << "Error: send name" << '\n';
						return;
					}

					sf::Packet id_packet;
					if(server->receive(id_packet) != sf::TcpSocket::Done)
					{
						std::cout << "Error: receive id" << '\n';
						return;
					}
					id_packet >> id_;

					server_ = std::make_unique<Connection>(std::move(server), 0, "Server");
					std::cout << "Successfully connected to server" << '\n';
				}
				else
					std::cout << "Error: connect to server" << '\n';
			}
			else
				std::cout << "IP address is incorrect" << '\n';

		}).detach();
}

Connection& ClientConnectionHandler::get_server() const
{
	return *server_;
}

int ClientConnectionHandler::get_id() const
{
	return id_;
}

void ServerConnectionHandler::listen_for_client_connection()
{
	const sf::IpAddress ip_address = sf::IpAddress::getLocalAddress();
	std::cout << "Local IP Address: " << ip_address << '\n';
	listen_ = true;
	std::thread([&]
		{
			if(listener_.listen(Connection::port) == sf::Socket::Done)
				std::cout << "Waiting for incoming connections... \n";
			else
				std::cout << "Error: listen on port " << Connection::port << '\n';

			//listener_.setBlocking(false);
			while (listen_)
			{
				if (auto client = std::make_unique<sf::TcpSocket>(); listener_.accept(*client) == sf::Socket::Done)
				{
					if (sf::Packet client_name; client->receive(client_name) == sf::Socket::Done)
					{
						std::string name;
						client_name >> name;
						int client_id = clients_.size() + 1;

						sf::Packet id_packet;
						id_packet << client_id;
						if(client->send(id_packet) != sf::Socket::Done)
						{
							std::cout << "Error: send data";
						}
						else
						{
							std::lock_guard guard(clients_mtx_);
							clients_.emplace_back(std::move(client), client_id, name);
							std::cout << "Connection established with: " << name << ", id = " << client_id << '\n';
						}
					}
					else
						std::cout << "Error: receive data \n";
				}
				//else
				//	std::cout << "Error: establish connection" << '\n';
			}
			std::cout << "Stop waiting for connections" << '\n';
			listener_.close();
		}).detach();
}

ServerConnectionHandler::~ServerConnectionHandler()
{
	stop_listen();
}

void ServerConnectionHandler::stop_listen()
{
	listen_ = false;
}

std::vector<Connection>& ServerConnectionHandler::get_connections()
{
	return clients_;
}
