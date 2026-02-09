#include "UDPTransmitter.h"
#include <iostream>
#include <chrono>

UDPTransmitter::UDPTransmitter()
    : sock(INVALID_SOCKET),
      serverPort(0),
      maxPacketSize(1400),
      running(false) {
    // 初始化Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
    }
}

UDPTransmitter::~UDPTransmitter() {
    stop();
    // 清理Winsock
    WSACleanup();
}

bool UDPTransmitter::initialize(const std::string& serverIP, unsigned int serverPort, unsigned int maxPacketSize) {
    this->serverIP = serverIP;
    this->serverPort = serverPort;
    this->maxPacketSize = maxPacketSize;
    
    // 创建UDP套接字
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        return false;
    }
    
    // 设置服务器地址
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    
    // 将IP地址转换为二进制格式
    if (inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr) != 1) {
        std::cerr << "Invalid address: " << serverIP << std::endl;
        closesocket(sock);
        sock = INVALID_SOCKET;
        return false;
    }
    
    // 设置套接字为非阻塞模式（可选，用于更好的流量控制）
    u_long mode = 1;
    if (ioctlsocket(sock, FIONBIO, &mode) != 0) {
        std::cerr << "Failed to set non-blocking mode: " << WSAGetLastError() << std::endl;
        // 继续执行，不强制要求非阻塞模式
    }
    
    running = true;
    return true;
}

bool UDPTransmitter::sendFrame(const UDPFrame& frame) {
    if (!running || sock == INVALID_SOCKET) {
        return false;
    }
    
    // 计算数据包大小
    UINT headerSize = sizeof(PacketHeader);
    UINT payloadSize = maxPacketSize - headerSize;
    
    // 计算分包数量
    UINT packetCount = (frame.data.size() + payloadSize - 1) / payloadSize;
    
    // 发送所有数据包
    for (UINT i = 0; i < packetCount; i++) {
        // 计算当前包的偏移量和大小
        UINT offset = i * payloadSize;
        UINT currentPayloadSize = std::min(payloadSize, (UINT)(frame.data.size() - offset));
        
        // 创建数据包
        std::vector<uint8_t> packet(headerSize + currentPayloadSize);
        
        // 填充包头
        PacketHeader* header = reinterpret_cast<PacketHeader*>(packet.data());
        header->frameId = frame.frameId;
        header->packetId = static_cast<uint16_t>(i);
        header->packetCount = static_cast<uint16_t>(packetCount);
        header->timestamp = frame.timestamp;
        
        // 填充负载
        memcpy(packet.data() + headerSize, frame.data.data() + offset, currentPayloadSize);
        
        // 发送数据包
        int result = sendto(
            sock,
            reinterpret_cast<const char*>(packet.data()),
            packet.size(),
            0,
            reinterpret_cast<const sockaddr*>(&serverAddr),
            sizeof(serverAddr)
        );
        
        if (result == SOCKET_ERROR) {
            int error = WSAGetLastError();
            // 非阻塞模式下的WSAEWOULDBLOCK错误是正常的，其他错误需要处理
            if (error != WSAEWOULDBLOCK) {
                std::cerr << "Failed to send packet: " << error << std::endl;
                // 直接返回失败，丢弃整个帧
                return false;
            }
        }
        
        // 简单的流量控制：如果发送缓冲区满了，稍微延迟一下
        if (result == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK) {
            // 短暂睡眠，避免CPU占用过高
            Sleep(1);
        }
    }
    
    return true;
}

void UDPTransmitter::stop() {
    running = false;
    
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
}