#include "SocketManager.h"
#include "SocketException.h"

void SocketManager::setSocketTCP(int adress_type)
{
	mysocket = socket(adress_type, SOCK_STREAM, IPPROTO_TCP);
	if (mysocket == INVALID_SOCKET) {
		throw SocketException(std::to_string(WSAGetLastError()));
	}

	this->serverAddr.sin_family = adress_type;
}

void SocketManager::ClSocket()
{
	if (this->isValid()) {
		closesocket(mysocket);
	}
}

bool SocketManager::isValid() const
{
	return !(mysocket == INVALID_SOCKET);
}

uint32_t SocketManager::getPort() const
{
	return ntohs(this->serverAddr.sin_port);
}

SocketManager::SocketManager()
{
	mysocket = INVALID_SOCKET;
	serverAddr = {};
}

SocketManager::~SocketManager()
{
	this->ClSocket();
}
