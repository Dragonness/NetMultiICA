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
	bool toSelf = false, toOthers = true;
	IpAddress ip;
	unsigned short port, ID;

	//string username;
	Vector2f pos;
	float speed, angle;
	friend Packet& operator<<(Packet pack, const ClientInfo& client)
	{
		pack << client.type;
		pack << client.toSelf;
		pack << client.toOthers;

		string ipString = client.ip.toString();
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

	friend Packet& operator>>(Packet pack, ClientInfo& client)
	{
		int type;
		pack >> type;
		//client.type = static_cast<Message>(type);

		pack >> client.toSelf;
		pack >> client.toOthers;

		string ipString;
		pack >> ipString;
		client.ip = IpAddress(ipString);

		pack >> client.port;
		pack >> client.ID;
		pack >> client.pos.x;
		pack >> client.pos.y;

		//pack >> client.username;

		pack >> client.speed;
		pack >> client.angle;
	}

	void PrintPos()
	{
		stringstream stream;
		stream << "X - " << pos.x << " Y - " << pos.y << endl;
		cout << stream.str();
	}
};