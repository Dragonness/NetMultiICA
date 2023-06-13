#pragma once
#include "util.h"

enum Message
{
	Setup, Update
};

class ClientInfo
{
private:
public:
	Message type;
	bool toSelf = false;
	bool toOthers = true; /// Used after getting some help from someone in class - useful for TCP server
	sf::IpAddress ip;
	unsigned short port;
	unsigned short ID;

	//string username;
	sf::Vector2f pos;
	float speed, angle;

	friend sf::Packet& operator<<(sf::Packet pack, const ClientInfo& client)
	{
		pack << client.type;
		pack << client.toSelf;
		pack << client.toOthers;

		std::string ipString = client.ip.toString();
		pack << ipString;

		pack << client.port;
		pack << client.ID;
		pack << client.pos.x;
		pack << client.pos.y;

		//pack << client.username;

		pack << client.speed;
		pack << client.angle;

		return pack;
	}

	friend sf::Packet& operator>>(sf::Packet pack, ClientInfo& client)
	{
		int type;
		pack >> type;
		//client.type = static_cast<Message>(type);

		pack >> client.toSelf;
		pack >> client.toOthers;

		std::string ipString;
		pack >> ipString;
		client.ip = sf::IpAddress(ipString);

		pack >> client.port;
		pack >> client.ID;
		pack >> client.pos.x;
		pack >> client.pos.y;

		//pack >> client.username;

		pack >> client.speed;
		pack >> client.angle;

		return pack;
	}

	void PrintPos()
	{
		std::stringstream stream;
		stream << "X - " << pos.x << " Y - " << pos.y << std::endl;
		std::cout << stream.str();
	}
};