#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

class UdpSender {
public:
    UdpSender(const char* ip, int port);
    ~UdpSender();

    bool initialize();
    bool send(const uint8_t* data, size_t size);
    void stop();

private:
    SOCKET sock;
    sockaddr_in serverAddr;
    std::string targetIp;
    int port;
    bool running;
};
