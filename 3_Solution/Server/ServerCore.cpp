#include "Logger.h"
#include "Exception.h"
#include "ClientSocket.h"
#include "ListeningSocket.h"
#include "DatabaseConection.h"
#include "ClientHandler.h"
#include "ServerCore.h"

#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define WSFAILED "WSAStartup failed to initialize: "
#define SVSTART  "Server started listening on port "
#define	CLICON   "Client conectat!"
#define BAYBAY   "#Server down; data has been cleaned.#"
#define FILELOG  "LogFile.txt"
#define SQLBASE  "SQL Server is ON: -> "


ServerCore* ServerCore::instance = nullptr;
ServerCore& ServerCore::getInstance()
{
	if (instance == nullptr) {
		try {
			Logger::SetFileLog(FILELOG);
			instance = new ServerCore();
		}
		catch (const Exception& e) {
			Logger::Error(e.what());
		}
	}
	return *instance;
}

ServerCore::ServerCore() : database(DatabaseConection::getInstance())
{
	this->active = false;
	Logger::SetFileLog(FILELOG);
	listener = new ListeningSocket();
}

ServerCore::~ServerCore()
{
	if (this->active) this->Quit();
	Logger::Info(BAYBAY);
	Logger::ClsFileLog();
	delete listener;
	delete& database;
}

void ServerCore::Init()
{
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		Logger::Error(WSFAILED + std::to_string(result) + "\n");
		return;
	}
	this->active = true;
	Logger::Info(SQLBASE + database._GetName_str());
}

void ServerCore::Quit()
{
	this->active = false;
	listener->ClSocket();
	WSACleanup();
}

void ServerCore::setup_Listener(int adress_type, int protocol)
{
	if (!active) return;
	try {
		if (protocol == TCP)
						//creaza un socket care foloseste TCP
			/*--------*/listener->setSocketTCP(adress_type);/*--------*/ 
	}
	catch (const Exception& e) {
		Logger::Error(e.what());
		this->Quit();
	}
}

void ServerCore::bind_Listener(uint32_t IP, uint16_t PORT)
{
	if (!active) return;
	try {
					//leaga sokcetul de port si IP
		/*--------*/listener->Binding(IP, PORT);/*--------*/ 
	}
	catch (const Exception& e) {
		Logger::Error(e.what());
		this->Quit();
	}
}

void ServerCore::start_Listener()
{
	if (!active) return;
	try {
					//pune socketul sa asculte
		/*--------*/listener->Listening();/*--------*/ 

		Logger::Info(SVSTART + std::to_string(listener->getPort()) + "\n");
	}
	catch (const Exception& e) {
		Logger::Error(e.what());
		this->Quit();
	}
}

void ServerCore::Run()
{
	if (!active) return;

	while (true) {
		try {

			ClientSocket* client = listener->Connecting();

			std::thread(ClientHandler(), std::move(*client)).detach();
		}
		catch (const Exception& e) {
			Logger::Warning(e.what());
		}
	}
}