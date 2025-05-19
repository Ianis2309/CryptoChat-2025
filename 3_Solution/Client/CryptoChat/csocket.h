#ifndef CSOCKET_H
#define CSOCKET_H
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>

#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <functional>
#include <thread>

enum class Header {
    BUFFER,
    _OK_,
    LOGIN,
    SIGNUP,
    TEXT,
    ERROR__,
    ADDCONT,
    AGENDA,
    TALKS,
    DELCONT,
    LOGOUT,
    PADDING
};

struct Contact {
    std::string username;
    std::string phone;
};


class CSocket
{
private:
    WSADATA wsaData;
    SOCKET sockfd;
    std::string server_ip;
    int server_port;

    void initWinsock();
    void createSocket();

    std::thread listenThread;
    bool keepListening = true;

    //void appendInt(std::string& packet, int value);
public:
    CSocket(const std::string& ip, int port) : sockfd(INVALID_SOCKET), server_ip(ip), server_port(port) {
        initWinsock();
        createSocket();
    }
    ~CSocket() {
        closeSocket();
        WSACleanup();
    }

    void connectToServer();

    void startListening(std::function<void(const std::string&)> callback);
    void stopListening();

    void sendLogIn(const std::string& phone, const std::string& pass);
    void sendSignUp(const std::string& phone, const std::string& username, const std::string& pass);

    void sendData(const std::string& data);

    Header parseHeader(const std::string& data);
    Header parseSecondHeader(const std::string& data);

    bool handleLoginOK(const std::string& data, std::string& outUsername);
    bool handleAgenda(const std::string& data, std::vector<Contact*>& outContacts);

    void sendLogOut();

    void sendAddContact(const std::string& phoneNumber);
    bool handleAddContactOK(const std::string& data, std::string& message);

    void sendText(const std::string& source, const std::string& dest ,const std::string message);
    bool handleText(const std::string& data, std::string& senderPhone, std::string& msg);

    void sendDelAccount(const std::string& phoneNumber);

    std::string receiveData();
    void closeSocket();
};

#endif
