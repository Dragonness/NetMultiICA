#include "util.h"
#include "TCPServer.h"

class TCPReceiver
{
private:
	std::shared_ptr<sf::TcpSocket> sharedTSock;
	std::list<sf::Packet> queue;
public:
	TCPReceiver(std::shared_ptr<sf::TcpSocket> TCPsockets, std::list<sf::Packet>& Tqueue) :
		sharedTSock(TCPsockets), queue(Tqueue) {}

	void ReceiveLoop()
	{
		std::cout << "The TCP receiver loop has been initiated\n";
		char buffer[256];

		while (1)
		{
			memset(buffer, 0, 256);
			size_t received = 0;
			sf::Packet packet;

			std::stringstream stream;
			stream << "TCP RECLOOP: " << sharedTSock->getRemoteAddress() << " : " << sharedTSock->getRemotePort() << std::endl;
			std::cout << stream.str();

			if (sharedTSock->receive(buffer, sizeof(buffer), received) == sf::Socket::Done)
			{
				packet.append(buffer, received);
				queue.push_back(packet);
			}
		}
	}
};

class Accepter
{
private:
	std::list<sf::Packet>& queue;
	std::list<std::shared_ptr<sf::TcpSocket>>& sockets;
public:
	Accepter(std::list<std::shared_ptr<sf::TcpSocket>> tSockets, std::list<sf::Packet> tQueue) :
		sockets(tSockets), queue(tQueue) {}

	void AcceptLoop()
	{
		std::cout << "The TCP accepter loop has been initiated\n";
		sf::TcpListener lsnr;

		if (lsnr.listen(TCPPORT) != sf::Socket::Done)
		{
			std::cerr << "TCP ERROR: Can't listen to accepter.\n";
			return;
		}
		std::cout << "TCP Server Bound to port: " << TCPPORT << std::endl;

		while (true)
		{
			std::shared_ptr<sf::TcpSocket> sock = std::make_shared<sf::TcpSocket>();

			if (lsnr.accept(*sock) != sf::Socket::Done)
			{
				std::cerr << "TCP ERROR: Connection not accepted.\n";
				return;
			}

			sockets.push_back(sock);
			std::stringstream stream;
			stream << "TCP Server accepted connection from: " << sock->getRemoteAddress() << " : " << sock->getRemotePort() << std::endl;
			std::cout << stream.str();

			TCPReceiver TCP_Rec(sock, queue);
			std::thread recThread(&TCPReceiver::ReceiveLoop, &TCP_Rec);
			recThread.detach();
		}
	}
};

void TCPServer()
{
	std::list<std::shared_ptr<sf::TcpSocket>> connectedClients;
	std::list<sf::Packet> queue;

	Accepter accept(connectedClients, queue);
	std::thread accThread(&Accepter::AcceptLoop, &accept);
	accThread.detach();

	while (1)
	{
		if (!queue.empty())
		{
			ClientInfo recInfo;
			sf::Packet recPack = queue.front();
			recPack >> recInfo;
			queue.pop_front();

			sf::Packet sendPack;
			ClientInfo sendInfo = recInfo;

			if (recInfo.type == Message::Setup)
			{
				sendInfo.ID = connectedClients.size() - 1;
				sendInfo.toSelf = true;
				sendInfo.toOthers = false;
			}
			sendPack << sendInfo;


			for (auto& client : connectedClients)
			{
				if (recInfo.toSelf && (client->getRemoteAddress() == recInfo.ip && client->getRemotePort() == recInfo.port))
				{
					client->send(sendPack.getData(), sendPack.getDataSize());
				}
				else if (recInfo.toOthers && !(client->getRemoteAddress() == recInfo.ip && client->getRemotePort() == recInfo.port))
				{
					client->send(sendPack.getData(), sendPack.getDataSize());
				}
			}
		}
	}
}