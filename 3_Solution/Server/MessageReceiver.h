#pragma once

class ProcessedMessage;
typedef ProcessedMessage PMS;

class MessageReceiver
{
	int bytesReceived;
	char* buffer;
	PMS* ptr_msg;

public:
	MessageReceiver(int bytesReceived, char* buffer);
	PMS* ParseHeader();
	bool ValidateHeader(int);

	void LoginCommand();
	void SignUpCommand();
	void MessagingCommand();
	void PushContCommand();
	void LogOutCommand();
	void PopContCommand();
};

