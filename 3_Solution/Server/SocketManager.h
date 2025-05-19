#pragma once
#include <cstdint>
#include <WinSock2.h>

class SocketManager
{
protected:
	SOCKET mysocket;
	sockaddr_in serverAddr;

public:
	void setSocketTCP(int);
	void ClSocket();
	bool isValid() const;

	uint32_t getPort() const;

	SocketManager();
	~SocketManager();

};


//int adress_type;
//uint16_t port;
//uint32_t ip;
