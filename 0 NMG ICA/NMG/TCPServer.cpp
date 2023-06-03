#include "util.h"

class TCPReceiver
{
private:
	shared_ptr<TcpSocket> sharedTSock;
	list<Packet> queue;
public:
	TCPReceiver(std::shared_ptr<TcpSocket> TCPsockets, list<Packet>& Tqueue) :
		sharedTSock(TCPsockets), queue(Tqueue) {};

	void ReceiveLoop()
	{
		cout << "The TCP receiver loop has been initiated\n";
		char buffer[256];

		while (1)
		{
			memset(buffer, 0, 256);
			size_t received = 0;
			Packet packet;

			stringstream ss;
			ss << "TCP RECLOOP: " << sharedTSock->getRemoteAddress() << " : " << sharedTSock->getRemotePort() << endl;

			cout << ss.str();

			if (sharedTSock->receive(buffer, sizeof(buffer), received) == Socket::Done)
			{
				packet.append(buffer, received);
				queue.push_back(packet);
			}
		}
	}
};