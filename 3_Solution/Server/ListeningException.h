#pragma once
#include "Exception.h"

class ListeningException : public Exception
{
public:
	ListeningException(const std::string& msj)
		: Exception("Listen failed: " + msj) {
	}
};

