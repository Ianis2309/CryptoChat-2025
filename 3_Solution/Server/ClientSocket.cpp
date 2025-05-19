#include "ClientSocket.h"


ClientSocket::ClientSocket(SOCKET&& socket, sockaddr_in&& Addr)
{
	this->mysocket = socket;
	this->serverAddr = Addr;
}

SOCKET* ClientSocket::get_SOCKET_ref()
{
	return &(this->mysocket);
}

int ClientSocket::Receiver(char buffer[], int len, int flags) const
{
	 int n = recv(this->mysocket, buffer, len, flags);
	 if (n < 0) return n;
	 buffer[n] = '\0'; // terminator de string
	 return n;
}

void ClientSocket::Sender(const char* buffer, int len, int flags) const
{
	send(mysocket, buffer, len, flags);
}