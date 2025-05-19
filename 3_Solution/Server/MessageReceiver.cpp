#include "ValidationException.h"
#include "ProcessedMessage.h"
#include "MessageReceiver.h"
#include "ClientSocket.h"
#include "HeaderType.h"


MessageReceiver::MessageReceiver(int bytesReceived, char* buffer)
{
	this->ptr_msg = nullptr;
	this->bytesReceived = bytesReceived;
	this->buffer = buffer;
}

PMS* MessageReceiver::ParseHeader()
{
	if (ValidateHeader(int(buffer[0])))
	{
		Header c = static_cast<Header>(buffer[0]);
		ptr_msg = new PMS();
		ptr_msg->setHeader(c);

		switch (c) {
		case Header::LOGIN:		LoginCommand();		break;
		case Header::SIGNUP:	SignUpCommand();	break;
		case Header::TEXT:		MessagingCommand(); break;
		case Header::ADDCONT:	PushContCommand();	break;
		case Header::LOGOUT:	LogOutCommand();	break;
		case Header::DELCONT:   PopContCommand();   break;
		default: 
			throw ValidationException("Header is incorrect.\n"); break;
		}

		return ptr_msg;
	}
	else {
		throw ValidationException("Header is no match.\n");
	}
	
	return nullptr;
}

bool MessageReceiver::ValidateHeader(int val)
{
	return (static_cast<int>(Header::BUFFER) <= val && val <= static_cast<int>(Header::PADDING));
}

void MessageReceiver::LoginCommand()
{
	std::string s(buffer + 1, buffer + 11);
	ptr_msg->setPhoneNumber(s);

	std::string p(buffer + 11, buffer + bytesReceived);
	ptr_msg->setPassword(p);
}

void MessageReceiver::SignUpCommand()
{
	std::string s(buffer + 1, buffer + 11);
	ptr_msg->setPhoneNumber(s);
	
	unsigned int number = buffer[11];
	ptr_msg->setNamelength(number);

	std::string u(buffer + 12, buffer + 12 + number);
	ptr_msg->setUsername(u);

	std::string p(buffer + 12 + number, buffer + bytesReceived);
	ptr_msg->setPassword(p);
}

void MessageReceiver::MessagingCommand()
{
	std::string s(buffer + 1, buffer + 11);
	ptr_msg->setSrc(s);

	std::string d(buffer + 11, buffer + 21);
	ptr_msg->setDest(d);

	std::string t(buffer + 21, buffer + bytesReceived);
	ptr_msg->setText(t);
}

void MessageReceiver::PushContCommand()
{
	std::string p(buffer + 1, buffer + 11);
	ptr_msg->setPhoneNumber(p);
}

void MessageReceiver::LogOutCommand() { }

void MessageReceiver::PopContCommand()
{
	std::string p(buffer + 1, buffer + 11);
	ptr_msg->setPhoneNumber(p);
}
