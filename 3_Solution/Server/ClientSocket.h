#pragma once
#include "SocketManager.h"



class ClientSocket : public SocketManager
{
public:
	ClientSocket() = default;
	ClientSocket(SOCKET&&, sockaddr_in&&);
	sockaddr_in* get_sockaddr_in_ref();
	SOCKET* get_SOCKET_ref();
	int Receiver(char[], int, int flags = 0) const;
	void Sender(const char*, int, int flags = 0) const;
};

