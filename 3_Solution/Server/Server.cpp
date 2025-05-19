#include "ServerCore.h"

#include <iostream>
#include <string>

#define IPv4_TYPE   IPv4_ADRESS
#define PROTOCOL    TCP
#define IP          ANY_IPv4
#define PORT        51234

void Run_Server(ServerCore& server)
{
    server.Init();

    server.setup_Listener(IPv4_TYPE, PROTOCOL);
    server.bind_Listener(IP, PORT);
    server.start_Listener();

    server.Run();

    server.Quit();
}

BOOL WINAPI ConsoleHandler(DWORD signal);

//#include "DatabaseConection.h"
using namespace std;

int main() {

    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {std::cerr << "Nu am putut seta handlerul pentru consola." << std::endl;return 1;}

    ServerCore& server = ServerCore::getInstance();

    Run_Server(server);

    delete& server;

    return 0;

   /* DatabaseConection& db = DatabaseConection::getInstance();

    int result = db.GetContactIDByPhone(1, "0123456789");
    cout << result;
     result = db.GetContactIDByPhone(1, "9876543210");
    cout << result;
     result = db.GetContactIDByPhone(1, "0727823976");
    cout << result;*/
}

BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_CLOSE_EVENT || signal == CTRL_LOGOFF_EVENT || signal == CTRL_SHUTDOWN_EVENT || signal == CTRL_C_EVENT) {

        ServerCore& server = ServerCore::getInstance();
        server.Quit();
        delete& server;

        return TRUE;
    }
    return FALSE;
}
