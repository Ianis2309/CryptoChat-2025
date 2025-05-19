#include "csocket.h"
#include <stdexcept>
#include <iostream>
#include <vector>
#include <QDebug>
//#include "crypto_utils.h"

void CSocket::initWinsock() {
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        throw std::runtime_error("WSAStartup failed with error: " + std::to_string(iResult));
    }
}

void CSocket::createSocket(){
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET) {
        throw std::runtime_error("Socket creation failed with error: " + std::to_string(WSAGetLastError()));
    }
}

/*void CSocket::appendInt(std::string& packet, int value){
    for(int i=0; i<4; i++)
        packet.push_back(static_cast<char>((value >> (8 * i)) & 0xFF));
}*/

void CSocket::connectToServer(){
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        throw std::runtime_error("Invalid IP address");
    }

    int ret = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret == SOCKET_ERROR) {
        int error_code = WSAGetLastError();
        closesocket(sockfd);
        sockfd = INVALID_SOCKET;
        throw std::runtime_error("Connect failed with error: " + std::to_string(error_code));
    }
}

void CSocket::startListening(std::function<void(const std::string&)> callback) {
    keepListening = true;

    listenThread = std::thread([this, callback]() {
        while (keepListening) {
            try {
                std::string data = receiveData();
                callback(data);
            } catch (const std::exception& e) {
                qDebug() << "Error while listening: " << e.what();
                break;
            }
        }
    });
    listenThread.detach();
}

void CSocket::stopListening(){
    keepListening = false;
}

Header CSocket::parseHeader(const std::string& data) {
    if (data.empty()) return Header::ERROR__;
    return static_cast<Header>(data[0]);
}

Header CSocket::parseSecondHeader(const std::string& data)
{
    if (data.empty()) return Header::ERROR__;
    return static_cast<Header>(data[1]);
}

bool CSocket::handleLoginOK(const std::string& data, std::string& outUsername) {
    if (parseHeader(data) != Header::_OK_) return false;
    outUsername = std::string(data.begin() + 1, data.end());
    return true;
}

bool CSocket::handleAgenda(const std::string& data, std::vector<Contact*>& outContacts) {
    if (parseHeader(data) != Header::AGENDA) return false;
    if (parseHeader(data) == Header::ERROR__) return true;

    size_t i = 1;
    while (i + 10 <= data.size()) {
        std::string phone = data.substr(i, 10);
        i += 10;

        std::string username;
        while (i < data.size() && static_cast<unsigned char>(data[i]) != 0xFF) {
            username += data[i];
            i++;
        }

        outContacts.push_back(new Contact{username, phone});
        if (i < data.size() && static_cast<unsigned char>(data[i]) == 0xFF) i++;
    }

    return true;
}

void CSocket::sendLogOut()
{
    std::string packet;
    packet.push_back(static_cast<char>(Header::LOGOUT));
    sendData(packet);
}

void CSocket::sendAddContact(const std::string& phoneNumber)
{
    std::string packet;
    packet.push_back(static_cast<char>(Header::ADDCONT));
    packet += phoneNumber;
    sendData(packet);
}

bool CSocket::handleAddContactOK(const std::string& data, std::string& username)
{
    if(parseHeader(data)!=Header::ADDCONT) return false;
    username = std::string(data.begin() + 1, data.end());
    return true;
}

std::string CSocket::receiveData()
{
    try
    {
        char buffer[4096] = {0};
        int bytes_received = recv(sockfd, buffer, sizeof(buffer), 0);

        if (bytes_received == SOCKET_ERROR)
        {
            throw std::runtime_error("Receive failed with error: " + std::to_string(WSAGetLastError()));
        }

        if (bytes_received == 0)
        {
            throw std::runtime_error("Server closed the connection.");
        }

         return std::string(buffer, bytes_received);
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Error in receiveData: " + std::string(e.what()));
    }
}

void CSocket::sendLogIn(const std::string& phone, const std::string& pass){
    std::string packet;
    packet.push_back(static_cast<char>(Header::LOGIN));

    packet+=phone;
    packet+=pass;

    sendData(packet);
}

void CSocket::sendSignUp(const std::string& phone, const std::string& username, const std::string& pass){
    std::string packet;
    packet.push_back(static_cast<char>(Header::SIGNUP));

    packet += phone;
    unsigned char lenght = username.length();
    packet.push_back(lenght);
    packet += username;
    packet += pass;

    sendData(packet);
}

void CSocket::sendText(const std::string& source, const std::string& dest ,const std::string message){
    std::string packet;
    packet.push_back(static_cast<char>(Header::TEXT));

    packet += source;
    packet += dest;
    //std::string encrypted = Crypt(message, dest);
    //packet += encrypted;

    packet += message;

    sendData(packet);
}

bool CSocket::handleText(const std::string& data, std::string& senderPhone, std::string& msg)
{
    if(parseHeader(data)!=Header::TEXT) return false;
    senderPhone = data.substr(1, 10);
    msg = data.substr(11);
    return true;
}

void CSocket::sendDelAccount(const std::string& phoneNumber)
{
    std::string packet;
    packet.push_back(static_cast<char>(Header::DELCONT));
    packet += phoneNumber;
    sendData(packet);
}

void CSocket::sendData(const std::string& data) {
    int bytesToSend = static_cast<int>(data.size());
    int ret = send(sockfd, reinterpret_cast<const char*>(data.c_str()), bytesToSend, 0);
    if (ret == SOCKET_ERROR) {
        throw std::runtime_error("Send failed with error: " + std::to_string(WSAGetLastError()));
    }
}

void CSocket::closeSocket() {
    if (sockfd != INVALID_SOCKET) {
        closesocket(sockfd);
        sockfd = INVALID_SOCKET;
    }
}
