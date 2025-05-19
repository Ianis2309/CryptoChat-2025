#pragma once
#include <WinSock2.h>
#include <cstdint>

#define IPv4_ADRESS AF_INET
#define TCP IPPROTO_TCP
#define ANY_IPv4 INADDR_ANY

class DatabaseConection;
typedef DatabaseConection& __DB__;
class ListeningSocket;
typedef ListeningSocket* LS_PTR;

class ServerCore
{
public:
	ServerCore(ServerCore&&) = delete;
	ServerCore(const ServerCore&) = delete;
	void operator=(const ServerCore&) = delete;

	static ServerCore& getInstance();
	~ServerCore();

	void Init();
	void Quit();
	void Run();
	void setup_Listener(int, int);
	void bind_Listener(uint32_t, uint16_t);
	void start_Listener();

private:
	static ServerCore* instance;
	ServerCore();

	LS_PTR listener;
	__DB__ database;
	bool active;

};

// _sock_man_;
