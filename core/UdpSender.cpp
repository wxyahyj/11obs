#include "UdpSender.h"
#include <iostream>
#include <cstring>

// 只在需要时包含Winsock头文件
#ifndef _WINSOCKAPI_
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

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
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }

    udpSocket = (void*)sock;

    // 设置socket为非阻塞模式
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);

    std::cout << "UDP socket created successfully" << std::endl;
    return true;
}

bool UdpSender::connectToServer() {
    // 创建服务器地址结构
    sockaddr_in* addr = new sockaddr_in();
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);

    // 转换IP地址
    int result = inet_pton(AF_INET, targetIp.c_str(), &addr->sin_addr);
    if (result <= 0) {
        std::cerr << "Invalid IP address: " << targetIp << std::endl;
        delete addr;
        return false;
    }

    serverAddr = (void*)addr;

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
    if (udpSocket) {
        SOCKET sock = (SOCKET)udpSocket;
        closesocket(sock);
        udpSocket = nullptr;
    }

    if (serverAddr) {
        delete (sockaddr_in*)serverAddr;
        serverAddr = nullptr;
    }

    WSACleanup();
    connected = false;

    std::cout << "UdpSender cleaned up" << std::endl;
}

bool UdpSender::sendPacket(const uint8_t* data, size_t size) {
    if (!connected || !udpSocket || !serverAddr) {
        return false;
    }

    SOCKET sock = (SOCKET)udpSocket;
    sockaddr_in* addr = (sockaddr_in*)serverAddr;

    // 发送数据包
    int bytesSentResult = sendto(
        sock,
        reinterpret_cast<const char*>(data),
        static_cast<int>(size),
        0,
        reinterpret_cast<sockaddr*>(addr),
        sizeof(sockaddr_in)
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
}

bool UdpSender::sendFrame(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        return false;
    }

    // 直接发送H.264裸流（Annex B格式）
    // 每个NAL单元以起始码（00 00 00 01或00 00 01）开头
    return sendPacket(data.data(), data.size());
}
