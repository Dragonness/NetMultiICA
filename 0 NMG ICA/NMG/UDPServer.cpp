#include "util.h"
#include "UDPServer.h"

struct Connection
{
	IpAddress IP;
	unsigned short port;
};

class UDPReceiver
{
private:
	UdpSocket* uSocket;
	list<Packet>& queue;
public:
	UDPReceiver(UdpSocket* UDPsock, list<Packet>& uQueue) :
		uSocket(UDPsock), queue(uQueue) {}


	void ReceiveLoop()
	{
		char buffer[256];
		
		while (1)
		{
			memset(buffer, 0, 256);
			size_t received = 0;
			Packet pack;
			IpAddress senderIp;
			unsigned short senderPort = UDPPORT;

			if (uSocket->receive(buffer, sizeof(buffer), received, senderIp, senderPort) == Socket::Done)
			{
				pack.append(buffer, received);
				queue.push_back(pack);
			}
		}
	}
};


void UDPServer()
{
	UdpSocket uSock;
	auto status = uSock.bind(UDPPORT);
	if (status != Socket::Done) { cerr << "ERROR: UDP socket not BOUND." << endl; return; }

	map<int, Connection> connedClients;
	list<Packet> queue;
	UDPReceiver udpRec(&uSock, queue);
	thread recThr(&UDPReceiver::ReceiveLoop, uSock);
	recThr.detach();

	while (true)
	{
		if (queue.size() != 0)
		{
			ClientInfo recInfo;
			Packet recPack = queue.front();
			recPack << recInfo;
			queue.pop_front();

			if (recInfo.type != Message::Update)
			{
				/// not the correct packet type
				continue;
			}


			bool senderFound = false;
			for (const auto& client : connedClients)
			{
				if (client.second.IP == recInfo.ip && client.second.port == recInfo.port)
				{
					senderFound = true;
					break;
				}
			}

			if (!senderFound)
			{
				Connection newConnect{ recInfo.ip, recInfo.port };
				connedClients.insert({ recInfo.ID, newConnect });
			}


			/// Sending relavant information to all clients except sender
			for (const auto& client : connedClients)
			{
				if (client.second.IP != recInfo.ip && client.second.port != recInfo.port)
				{
					uSock.send(recPack.getData(), recPack.getDataSize(), client.second.IP, client.second.port);
				}
			}
		}
	}
}