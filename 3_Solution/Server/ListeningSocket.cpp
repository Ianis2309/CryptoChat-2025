#include "ClientSocket.h"
#include "ListeningSocket.h"
#include "BindingException.h"
#include "ListeningException.h"
#include "ConectingException.h"


void ListeningSocket::Binding(uint32_t IP, uint16_t PORT)
{
	this->serverAddr.sin_addr.s_addr = IP;
	this->serverAddr.sin_port = htons(PORT);

	int result = bind(mysocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

	if (result == SOCKET_ERROR){
		throw BindingException(std::to_string(WSAGetLastError()));
	}

}

void ListeningSocket::Listening()
{
	int result = listen(mysocket, SOMAXCONN);

	if (result == SOCKET_ERROR){
		throw ListeningException(std::to_string(WSAGetLastError()));
	}

}

ClientSocket* ListeningSocket::Connecting()
{
	sockaddr_in adres;
	int clientSize = sizeof(adres);
	SOCKET sock = accept(this->mysocket, (sockaddr*)&adres, &clientSize);
	ClientSocket* socket_caught = new ClientSocket(std::move(sock), std::move(adres));
	
	if (!socket_caught->isValid()) {
		throw ConectingException(std::to_string(WSAGetLastError()));
	}

	return socket_caught;
}

