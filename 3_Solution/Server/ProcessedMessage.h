#pragma once
#include "HeaderType.h"

#include <vector>
#include <string>
#include <cstdint>

class ProcessedMessage
{
	Header header;
	std::string phonenumber;
	std::string password;
	std::string username;
	int namelength;
	std::string dest;
	std::string src;
	std::string text;

public:
	void setHeader(Header);
	void setPhoneNumber(std::string);
	void setPassword(std::string);
	void setUsername(std::string);
	void setNamelength(int);
	void setDest(std::string);
	void setSrc(std::string);
	void setText(const std::string&);

	Header getHeader() const;
	std::string getPhoneNumber() const;
	std::string getPassword() const;
	std::string getUsername() const;
	int getNamelength() const;
	std::string getDest() const;
	std::string getSrc() const;
	const std::string& getText() const;


	ProcessedMessage();
};

