#pragma once
#include "Exception.h"

class SystemExeption : public Exception
{
public:
	SystemExeption(const std::string& msj)
		: Exception("Partial Outage Notification: " + msj) {
	}
};

