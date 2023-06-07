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
		uSocket(UDPsock), queue(uQueue) {};


	void ReceiveLoop()
	{
		char buffer[256];
		
		while (1)
		{
			std::memset(buffer, 0, 256); /// Resetting buffer
			std::size_t received = 0;
			sf::Packet pack;
			sf::IpAddress senderIp;
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
	sf::UdpSocket uSock;
	auto status = uSock.bind(UDPPORT);
	if (status != Socket::Done) { cerr << "ERROR: UDP socket not BOUND." << endl; return; }

	std::map<int, Connection> connedClients;
	std::list<Packet> sQueue;
	UDPReceiver udpRec(&uSock, sQueue);
	thread recThr(&UDPReceiver::ReceiveLoop, udpRec);
	recThr.detach();

	while (true)
	{
		if (sQueue.size() != 0)
		{
			ClientInfo recInfo;
			sf::Packet recPack = sQueue.front(); // fucks up smth or other
			recPack << recInfo;
			sQueue.pop_front();

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