#pragma once
#include "Exception.h"

class ConectingException : public Exception
{
public:
	ConectingException(const std::string& msj)
		: Exception("Accept failed: " + msj) {
	}
};

