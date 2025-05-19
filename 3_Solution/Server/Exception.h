#pragma once

#include <string>
#include <stdexcept>

class Exception : public std::exception
{
private:
	std::string mesaj;

public:
	Exception(const std::string& msj) : mesaj(msj) {}
	const char* what() const noexcept override { return mesaj.c_str(); }
};

