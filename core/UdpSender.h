#pragma once

#include <string>
#include <vector>
#include <cstdint>

// 确保Winsock头文件正确包含
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#endif

class UdpSender {
public:
    UdpSender();
    ~UdpSender();

    bool initialize(const std::string& targetIp, int port);
    void cleanup();

    bool sendFrame(const std::vector<uint8_t>& data);
    bool sendPacket(const uint8_t* data, size_t size);

    bool isConnected() const { return connected; }
    int getBytesSent() const { return bytesSent; }
    int getPacketsSent() const { return packetsSent; }

private:
    bool createSocket();
    bool resolveAddress();

private:
    std::string targetIp;
    int port = 0;

#ifdef _WIN32
    SOCKET udpSocket = INVALID_SOCKET;
    sockaddr_in serverAddr;
    int serverAddrSize = sizeof(serverAddr);
#else
    int udpSocket = -1;
    struct sockaddr_in serverAddr;
    socklen_t serverAddrSize = sizeof(serverAddr);
#endif

    bool connected = false;

    // 统计信息
    int bytesSent = 0;
    int packetsSent = 0;
};
