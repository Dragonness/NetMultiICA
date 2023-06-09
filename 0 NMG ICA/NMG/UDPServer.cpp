#include "util.h"
#include "UDPServer.h"

struct Connection
{
	sf::IpAddress IP;
	unsigned short port;
};

class UDPReceiver
{
private:
	sf::UdpSocket* uSocket;
	std::list<sf::Packet>& queue;
public:
	UDPReceiver(sf::UdpSocket* UDPsock, std::list<sf::Packet>& uQueue) :
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

			if (uSocket->receive(buffer, sizeof(buffer), received, senderIp, senderPort) == sf::Socket::Done)
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
	if (status != sf::Socket::Done) 
	{ 
		std::cerr << "ERROR: UDP socket not BOUND." << std::endl;
		return; 
	}

	std::map<int, Connection> connedClients;
	std::list<sf::Packet> sQueue;
	UDPReceiver udpRec(&uSock, sQueue);

	std::thread recThr(&UDPReceiver::ReceiveLoop, udpRec);
	recThr.detach();

	while (true)
	{
		if (sQueue.size() >= 1)
		{
			ClientInfo recInfo;
			sf::Packet recPack = sQueue.front(); // Keeps throwing exceptions 
			// ^^ says the queue is empty but it shouldn't even get here if the queue is empty
			recPack >> recInfo;
			sQueue.pop_front();

			if (recInfo.type != Message::Update)
			{
				/// not the correct packet type
				// skip it
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
				if (client.second.IP != recInfo.ip || client.second.port != recInfo.port)
				{
					uSock.send(recPack.getData(), recPack.getDataSize(), client.second.IP, client.second.port);
				}
			}
		}
	}
}