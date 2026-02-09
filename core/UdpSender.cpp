#include "UdpSender.h"
#include <iostream>
#include <cstring>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

UdpSender::UdpSender() {
}

UdpSender::~UdpSender() {
    cleanup();
}

bool UdpSender::createSocket() {
    // 初始化Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return false;
    }

    // 创建UDP socket
    socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }

    // 设置socket为非阻塞模式
    u_long mode = 1;
    ioctlsocket(socket, FIONBIO, &mode);

    std::cout << "UDP socket created successfully" << std::endl;
    return true;
}

bool UdpSender::connectToServer() {
    // 设置服务器地址
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
}

bool UdpSender::initialize(const std::string& ip, int p) {
    targetIp = ip;
    port = p;

    if (!createSocket()) {
        return false;
    }

    if (!connectToServer()) {
        return false;
    }

    connected = true;
    std::cout << "UdpSender initialized successfully" << std::endl;
    return true;
}

void UdpSender::cleanup() {
    if (socket != INVALID_SOCKET) {
        closesocket(socket);
        socket = INVALID_SOCKET;
    }

    WSACleanup();
    connected = false;

    std::cout << "UdpSender cleaned up" << std::endl;
}

bool UdpSender::sendPacket(const uint8_t* data, size_t size) {
    if (!connected || socket == INVALID_SOCKET) {
        return false;
    }

    // 发送数据包
    int bytesSent = sendto(
        socket,
        reinterpret_cast<const char*>(data),
        static_cast<int>(size),
        0,
        reinterpret_cast<sockaddr*>(&serverAddr),
        sizeof(serverAddr)
    );

    if (bytesSent == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            std::cerr << "Failed to send packet: " << error << std::endl;
        }
        return false;
    }

    // 更新统计信息
    this->bytesSent += bytesSent;
    packetsSent++;

    return true;
}

bool UdpSender::sendFrame(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        return false;
    }

    // 直接发送H.264裸流（Annex B格式）
    // 每个NAL单元以起始码（00 00 00 01或00 00 01）开头
    return sendPacket(data.data(), data.size());
}
