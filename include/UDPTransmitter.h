#pragma once

// 强制包含winsock2.h并防止winsock.h被包含
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_

#include <cstdint>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <atomic>
#include <string>

#pragma comment(lib, "ws2_32.lib")

class UDPTransmitter {
private:
    SOCKET sock;
    sockaddr_in serverAddr;
    
    // 配置参数
    std::string serverIP;
    unsigned int serverPort;
    unsigned int maxPacketSize;
    
    std::atomic<bool> running;
    
public:
    struct UDPFrame {
        std::vector<uint8_t> data;
        uint32_t frameId;
        uint64_t timestamp;
    };
    
    // UDP数据包结构
    struct PacketHeader {
        uint32_t frameId;      // 全局唯一帧标识符
        uint16_t packetId;     // 当前分包序号
        uint16_t packetCount;  // 当前帧总包数
        uint64_t timestamp;    // 微秒级时间戳
    };
    
    UDPTransmitter();
    ~UDPTransmitter();
    
    bool initialize(const std::string& serverIP, unsigned int serverPort, unsigned int maxPacketSize = 1400);
    bool sendFrame(const UDPFrame& frame);
    bool sendFrame(const std::vector<uint8_t>& data);
    void stop();
    
    const std::string& getServerIP() const { return serverIP; }
    unsigned int getServerPort() const { return serverPort; }
    unsigned int getMaxPacketSize() const { return maxPacketSize; }
};