#include "ProcessedMessage.h"

void ProcessedMessage::setHeader(Header header)
{
	this->header = header;
}

void ProcessedMessage::setPhoneNumber(std::string phonenumber)
{
	this->phonenumber = phonenumber;
}

void ProcessedMessage::setPassword(std::string password)
{
	this->password = password;
}

void ProcessedMessage::setUsername(std::string username)
{
	this->username = username; 
}

void ProcessedMessage::setNamelength(int namelength)
{
	this->namelength = namelength;
}

void ProcessedMessage::setDest(std::string dest)
{
	this->dest = dest;
}

void ProcessedMessage::setSrc(std::string src)
{
	this->src = src;
}

void ProcessedMessage::setText(const std::string& text)
{
	this->text = text;
}

Header ProcessedMessage::getHeader() const
{
	return header;
}

std::string ProcessedMessage::getPhoneNumber() const
{
	return phonenumber;
}

std::string ProcessedMessage::getPassword() const
{
	return password;
}

std::string ProcessedMessage::getUsername() const
{
	return username;
}

int ProcessedMessage::getNamelength() const
{
	return namelength;
}

std::string ProcessedMessage::getDest() const
{
	return dest;
}

std::string ProcessedMessage::getSrc() const
{
	return src;
}

const std::string& ProcessedMessage::getText() const
{
	return text;
}

ProcessedMessage::ProcessedMessage()
{
	header = Header::PADDING;
	namelength = 0;
}
