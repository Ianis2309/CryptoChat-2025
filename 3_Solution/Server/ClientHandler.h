#pragma once
#include "ClientSocket.h"

class CustomerTable;
class MessageReceiver;
class ProcessedMessage;
class DatabaseConection;
typedef ClientSocket Client;
typedef CustomerTable* Table_PTR;
typedef MessageReceiver* MSG_PTR;
typedef ProcessedMessage* PMS_PTR;
typedef DatabaseConection* DB_PTR;


class ClientHandler
{
public:
	void operator() (Client&&);
	void ProcessSignal();
	void HandlesMassege(char*, int);
	void NiceOUT();
	void forcExit();
	void ExecuteCommand(PMS_PTR);
	
	void ClientLogin();
	void ClientSignUp();
	void CustomerMessaging();
	void AddContact();
	void DeleteContact();
	void GetOut();
	
	void ConfirmationResponse(const std::string&, char h = 1);
	void WarningResponse(const std::string&);
	void QuickResponse(char stat, char flag);
	void Status_Turn(bool);
	void RespondClient(int);
	void RestartClient();
	void ClientDisconected();

private:
	int UserId;
	ClientSocket client;
	Table_PTR table;
	MSG_PTR receiver;
	PMS_PTR mp;
	DB_PTR db;

	void CreateTalkFile(int);
	std::string CatFileName(int);
	void LoadAgenda() const;
	

	int MAX(int a, int b) {return (a < b ? b : a);}
	int MIN(int a, int b) {return (a > b ? b : a);}
};