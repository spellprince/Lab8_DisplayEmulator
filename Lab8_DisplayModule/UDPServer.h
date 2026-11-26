#pragma once

#include <winsock2.h>
#include <thread>
#include <atomic>

class UDPServer
{
public:
    UDPServer();
    ~UDPServer();

    bool start();
    void stop();

private:
    SOCKET serverSocket;
    std::thread serverThread;
    std::atomic<bool> running;

    void receiveLoop();
};