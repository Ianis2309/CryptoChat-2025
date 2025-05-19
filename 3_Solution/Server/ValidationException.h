#pragma once
#include "Exception.h"

class ValidationException : public Exception
{
public:
	ValidationException(const std::string& msj)
		: Exception("The data received is invalid: " + msj) {
	}
};

