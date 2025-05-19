#pragma once
#include "Exception.h"

class DatabaseException : public Exception
{
public:
	DatabaseException(const std::string& msj)
		: Exception("The database : " + msj) {
	}
};

