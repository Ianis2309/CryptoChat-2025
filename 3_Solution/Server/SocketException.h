#pragma once
#include "Exception.h"

class SocketException : public Exception
{
public:
	SocketException(const std::string& msj)
		: Exception("Socket creation failed: " + msj) {}
};

