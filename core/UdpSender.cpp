#include "UdpSender.h"
#include <iostream>
#include <cstring>
#include <stdexcept>

UdpSender::UdpSender()
    : udpSocket(INVALID_SOCKET),
      connected(false),
      bytesSent(0),
      packetsSent(0)
{
    // 初始化Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        // 注意：这里不抛出异常，因为构造函数不应该抛出异常
    }
}

UdpSender::~UdpSender() {
    try {
        cleanup();
    } catch (const std::exception& e) {
        std::cerr << "Error in UdpSender destructor: " << e.what() << std::endl;
    }
}

bool UdpSender::createSocket() {
    try {
        // 创建UDP socket
        udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (udpSocket == INVALID_SOCKET) {
            int error = WSAGetLastError();
            std::cerr << "Failed to create socket: " << error << std::endl;
            return false;
        }

        // 设置socket为非阻塞模式
        u_long mode = 1;
        int result = ioctlsocket(udpSocket, FIONBIO, &mode);
        if (result == SOCKET_ERROR) {
            int error = WSAGetLastError();
            std::cerr << "Failed to set non-blocking mode: " << error << std::endl;
            closesocket(udpSocket);
            udpSocket = INVALID_SOCKET;
            return false;
        }

        std::cout << "UDP socket created successfully" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error creating socket: " << e.what() << std::endl;
        if (udpSocket != INVALID_SOCKET) {
            closesocket(udpSocket);
            udpSocket = INVALID_SOCKET;
        }
        return false;
    }
}

bool UdpSender::resolveAddress() {
    try {
        // 初始化服务器地址结构
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);

        // 转换IP地址
        int result = inet_pton(AF_INET, targetIp.c_str(), &serverAddr.sin_addr);
        if (result <= 0) {
            std::cerr << "Invalid IP address: " << targetIp << std::endl;
            return false;
        }

        std::cout << "UDP sender configured for " << targetIp << ":" << port << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error resolving address: " << e.what() << std::endl;
        return false;
    }
}

bool UdpSender::initialize(const std::string& ip, int p) {
    try {
        if (connected) {
            std::cerr << "UDP sender already initialized" << std::endl;
            return false;
        }

        targetIp = ip;
        port = p;

        // 创建socket
        if (!createSocket()) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }

        // 解析地址
        if (!resolveAddress()) {
            std::cerr << "Failed to resolve address" << std::endl;
            cleanup();
            return false;
        }

        connected = true;
        std::cout << "UdpSender initialized successfully" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error initializing UDP sender: " << e.what() << std::endl;
        cleanup();
        return false;
    }
}

void UdpSender::cleanup() {
    try {
        if (udpSocket != INVALID_SOCKET) {
            closesocket(udpSocket);
            udpSocket = INVALID_SOCKET;
        }

        // 重置统计信息
        bytesSent = 0;
        packetsSent = 0;
        connected = false;

        std::cout << "UdpSender cleaned up" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error cleaning up UDP sender: " << e.what() << std::endl;
    }
}

bool UdpSender::sendPacket(const uint8_t* data, size_t size) {
    try {
        if (!connected || udpSocket == INVALID_SOCKET) {
            std::cerr << "UDP sender not initialized" << std::endl;
            return false;
        }

        if (data == nullptr || size == 0) {
            std::cerr << "Invalid data to send" << std::endl;
            return false;
        }

        // 发送数据包
        int bytesSentResult = sendto(
            udpSocket,
            reinterpret_cast<const char*>(data),
            static_cast<int>(size),
            0,
            reinterpret_cast<sockaddr*>(&serverAddr),
            serverAddrSize
        );

        if (bytesSentResult == SOCKET_ERROR) {
            int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK) {
                std::cerr << "Failed to send packet: " << error << std::endl;
            }
            return false;
        }

        // 更新统计信息
        bytesSent += bytesSentResult;
        packetsSent++;

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error sending packet: " << e.what() << std::endl;
        return false;
    }
}

bool UdpSender::sendFrame(const std::vector<uint8_t>& data) {
    try {
        if (data.empty()) {
            std::cerr << "Empty data to send" << std::endl;
            return false;
        }

        return sendPacket(data.data(), data.size());
    } catch (const std::exception& e) {
        std::cerr << "Error sending frame: " << e.what() << std::endl;
        return false;
    }
}
