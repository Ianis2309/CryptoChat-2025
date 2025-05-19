#pragma once
#include "SocketManager.h"

class ClientSocket;


class ListeningSocket : public SocketManager
{
public:
	void Binding(uint32_t, uint16_t);
	void Listening();
	ClientSocket* Connecting();
};

