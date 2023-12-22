#include "ConnectionHandler.h"

Connection::Connection(std::unique_ptr<sf::TcpSocket>&& socket, const int id, std::string name) :
socket_(std::move(socket)), id_(id), name_(std::move(name))
{	}

sf::TcpSocket& Connection::get_socket() const
{
	return *socket_;
}

int Connection::get_id() const
{
	return id_;
}

std::string Connection::get_name() const
{
	return name_;
}

void Connection::put_input(const Input& input)
{
	std::lock_guard guard(input_mtx_);
	input_ = input;
}

std::optional<Input> Connection::get_input()
{
	std::lock_guard guard(input_mtx_);
	const auto input = input_;
	input_ = {};
	return input;

}

void Connection::put_update(const std::shared_ptr<sf::Packet>& update)
{
	std::lock_guard guard(update_mtx_);
	update_ = update;
}

std::shared_ptr<sf::Packet> Connection::get_update()
{
	std::lock_guard guard(update_mtx_);
	const auto update = update_;
	update_.reset();
	return update;
}

//for client
void Connection::start_send_input()
{
	send_input_active_ = true;
	std::thread([&]
		{
			while (send_input_active_)
			{
				if (input_)
				{
					Input input;
					{
						std::lock_guard guard(input_mtx_);
						input = *input_;
						input_ = {};
					}
					sf::Packet packet;
					input.write_to_packet(packet);
					socket_->send(packet);
					//std::cout << "Packet sent " << input.a << ' ' << input.d << '\n';
				}
			}
		}).detach();
}

void Connection::stop_send_input()
{
	send_input_active_ = false;
}

//for server
void Connection::start_receive_input()
{
	receive_input_active_ = true;
	std::thread([&]
		{
			while (receive_input_active_)
			{
				sf::Packet packet;
				socket_->receive(packet);
				Input input;
				input.read_from_packet(packet);
				//std::cout << "Packet received " << input.a << ' ' << input.d <<'\n';

				std::lock_guard guard(input_mtx_);
				input_ = input;
			}
		}).detach();
}

void Connection::stop_receive_input()
{
	receive_input_active_ = false;
}

void Connection::start_send_updates()
{
	send_updates_active_ = true;
	std::thread([&]
		{
			while (send_updates_active_)
			{
				std::shared_ptr<sf::Packet> packet;
				{
					std::lock_guard guard(update_mtx_);
					packet = update_;
					update_.reset();
				}
				if(packet != nullptr)
					socket_->send(*packet);
			}	
		}
	).detach();
}

void Connection::stop_send_updates()
{
	send_updates_active_ = false;
}

void Connection::start_receive_updates()
{
	receive_updates_active_ = true;
	std::thread([&]
		{
			while (receive_updates_active_)
			{
				auto packet = std::make_shared<sf::Packet>();
				socket_->receive(*packet);

				std::lock_guard guard(update_mtx_);
				update_ = packet;
			}
		}).detach();
}

void Connection::stop_receive_updates()
{
	receive_updates_active_ = false;
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

void ClientConnectionHandler::put_input_to_server(const Input& input) const
{
	server_->put_input(input);
}

std::shared_ptr<sf::Packet> ClientConnectionHandler::get_update_from_server() const
{
	return server_->get_update();
}

void ClientConnectionHandler::start_send_input() const
{
	server_->start_send_input();
}

void ClientConnectionHandler::stop_send_input() const
{
	server_->stop_receive_input();
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void ClientConnectionHandler::start_receive_updates() const
{
	server_->start_receive_updates();
}

void ClientConnectionHandler::stop_receive_updates() const
{
	server_->stop_receive_updates();
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
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

std::list<Connection>& ServerConnectionHandler::get_connections()
{
	return clients_;
}

void ServerConnectionHandler::read_player_name()
{
	std::cout << "Input your name: \n";
	std::cin >> player_name_;
}

std::string ServerConnectionHandler::get_player_name() const
{
	return player_name_;
}

void ServerConnectionHandler::start_receive_input()
{
	for (auto& client : clients_)
		client.start_receive_input();
}

void ServerConnectionHandler::stop_receive_input()
{
	for (auto& client : clients_)
		client.stop_receive_input();
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void ServerConnectionHandler::start_send_updates()
{
	for (auto& client : clients_)
		client.start_send_updates();
}

void ServerConnectionHandler::stop_send_updates()
{
	for (auto& client : clients_)
		client.stop_send_updates();
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

std::vector<std::optional<Input>> ServerConnectionHandler::get_clients_input()
{
	std::vector<std::optional<Input>> clients_input;
	for (auto& client : clients_)
		clients_input.push_back(client.get_input());
	return clients_input;
}

void ServerConnectionHandler::put_update_to_clients(const sf::Packet& update_packet)
{
	const auto shared_packet = std::make_shared<sf::Packet>(update_packet);
	for (auto& client : clients_)
		client.put_update(shared_packet);
}
