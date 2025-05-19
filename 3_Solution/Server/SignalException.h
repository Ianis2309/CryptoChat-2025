#pragma once
#include "Exception.h"

class SignalException : public Exception
{
public:
	SignalException(const std::string& msj)
		: Exception("Signal status: " + msj) {
	}
};

