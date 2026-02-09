#pragma once

#include <string>
#include <vector>
#include <cstdint>

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
    bool connectToServer();

private:
    std::string targetIp;
    int port = 0;

    SOCKET socket = INVALID_SOCKET;
    sockaddr_in serverAddr;

    bool connected = false;

    // 统计信息
    int bytesSent = 0;
    int packetsSent = 0;
};
