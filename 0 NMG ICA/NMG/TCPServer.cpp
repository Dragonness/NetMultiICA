#include "util.h"
#include "TCPServer.h"

class TCPReceiver
{
private:
	shared_ptr<TcpSocket> sharedTSock;
	list<Packet> queue;
public:
	TCPReceiver(std::shared_ptr<TcpSocket> TCPsockets, list<Packet>& Tqueue) :
		sharedTSock(TCPsockets), queue(Tqueue) {}

	void ReceiveLoop()
	{
		cout << "The TCP receiver loop has been initiated\n";
		char buffer[256];

		while (1)
		{
			memset(buffer, 0, 256);
			size_t received = 0;
			Packet packet;

			stringstream stream;
			stream << "TCP RECLOOP: " << sharedTSock->getRemoteAddress() << " : " << sharedTSock->getRemotePort() << endl;
			cout << stream.str();

			if (sharedTSock->receive(buffer, sizeof(buffer), received) == Socket::Done)
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
	list<Packet>& queue;
	list<shared_ptr<TcpSocket>>& sockets;
public:
	Accepter(list<shared_ptr<TcpSocket>> tSockets, list<Packet> tQueue) :
		sockets(tSockets), queue(tQueue) {}

	void AcceptLoop()
	{
		cout << "The TCP accepter loop has been initiated\n";
		TcpListener lsnr;

		if (lsnr.listen(TCPPORT) != Socket::Done)
		{
			cerr << "TCP Error: Can't listen to accepter.";
			return;
		}
		cout << "TCP Server Bound to port: " << TCPPORT << endl;

		while (true)
		{
			shared_ptr<TcpSocket> sock = make_shared<TcpSocket>();

			if (lsnr.accept(*sock) != Socket::Done)
			{
				cerr << "TCP Error: Connection not accepted." << endl;
				return;
			}

			sockets.push_back(sock);
			stringstream stream;
			stream << "TCP Server accepted connection from: " << sock->getRemoteAddress() << " : " << sock->getRemotePort() << endl;
			cout << stream.str();

			TCPReceiver TCP_Rec(sock, queue);
			thread recThread(&TCPReceiver::ReceiveLoop, &TCP_Rec);
			recThread.detach();
		}
	}
};

void TCPServer()
{
	std::list<shared_ptr<TcpSocket>> connectedClients;
	std::list<Packet> queue;

	Accepter accept(connectedClients, queue);
	std::thread accThread(&Accepter::AcceptLoop, &accept);
	accThread.detach();

	while (1)
	{
		if (!queue.empty())
		{
			ClientInfo recInfo;
			Packet recPack = queue.front();
			recPack >> recInfo;
			queue.pop_front();

			Packet sendPack;
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