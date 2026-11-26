#include "UDPServer.h"
#include "DisplayProtocol.h"

#include <iostream>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

extern void executeCommand(const Command& cmd);

UDPServer::UDPServer()
{
    serverSocket = INVALID_SOCKET;
    running = false;
}

UDPServer::~UDPServer()
{
    stop();
}

bool UDPServer::start()
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return false;

    serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (serverSocket == INVALID_SOCKET)
    {
        WSACleanup();
        return false;
    }

    sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_port = htons(12345);
    address.sin_addr.s_addr = INADDR_ANY;

    if (bind(
        serverSocket,
        reinterpret_cast<sockaddr*>(&address),
        sizeof(address)) == SOCKET_ERROR)
    {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
        WSACleanup();
        return false;
    }

    running = true;
    serverThread = std::thread(&UDPServer::receiveLoop, this);

    std::cout << "UDP server started on port 12345" << std::endl;

    return true;
}

void UDPServer::stop()
{
    if (!running)
        return;

    running = false;

    closesocket(serverSocket);
    serverSocket = INVALID_SOCKET;

    if (serverThread.joinable())
        serverThread.join();

    WSACleanup();
}

void UDPServer::receiveLoop()
{
    uint8_t buffer[65507];

    while (running)
    {
        int received = recvfrom(
            serverSocket,
            reinterpret_cast<char*>(buffer),
            sizeof(buffer),
            0,
            nullptr,
            nullptr
        );

        if (received <= 0)
            continue;

        std::vector<uint8_t> data(
            buffer,
            buffer + received
        );

        Command cmd;

        if (DisplayProtocol::parse(data, cmd))
            executeCommand(cmd);
    }
}