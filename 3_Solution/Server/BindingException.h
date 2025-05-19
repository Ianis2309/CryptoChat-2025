#pragma once
#include "Exception.h"

class BindingException : public Exception
{
public:
	BindingException(const std::string& msj)
		: Exception("Bind failed: " + msj) {
	}
};

