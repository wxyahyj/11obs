#include "UdpSender.h"
#include <iostream>
#include <cstring>

UdpSender::UdpSender(const char* ip, int port)
    : sock(INVALID_SOCKET),
      targetIp(ip),
      port(port),
      running(false) {
    // 初始化Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
    }
}

UdpSender::~UdpSender() {
    stop();
    // 清理Winsock
    WSACleanup();
}

bool UdpSender::initialize() {
    // 创建UDP套接字
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        return false;
    }

    // 设置服务器地址
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    // 将IP地址转换为二进制格式
    if (inet_pton(AF_INET, targetIp.c_str(), &serverAddr.sin_addr) != 1) {
        std::cerr << "Invalid address: " << targetIp << std::endl;
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

bool UdpSender::send(const uint8_t* data, size_t size) {
    if (!running || sock == INVALID_SOCKET) {
        return false;
    }

    // 最大数据包大小
    const size_t maxPacketSize = 1400;
    const size_t headerSize = 12; // 简单的包头：frameId(4) + packetId(2) + packetCount(2) + timestamp(4)
    const size_t payloadSize = maxPacketSize - headerSize;

    // 计算分包数量
    size_t packetCount = (size + payloadSize - 1) / payloadSize;

    // 生成帧ID和时间戳
    static uint32_t frameIdCounter = 0;
    uint32_t frameId = frameIdCounter++;
    uint64_t timestamp = 0; // 可以使用更精确的时间戳

    // 发送所有数据包
    for (size_t i = 0; i < packetCount; i++) {
        // 计算当前包的偏移量和大小
        size_t offset = i * payloadSize;
        size_t currentPayloadSize = (i == packetCount - 1) ? (size - offset) : payloadSize;
        size_t packetSize = headerSize + currentPayloadSize;

        // 创建数据包
        std::vector<uint8_t> packet(packetSize);

        // 填充包头
        uint8_t* header = packet.data();
        memcpy(header, &frameId, 4);
        uint16_t packetId = static_cast<uint16_t>(i);
        memcpy(header + 4, &packetId, 2);
        uint16_t totalPackets = static_cast<uint16_t>(packetCount);
        memcpy(header + 6, &totalPackets, 2);
        memcpy(header + 8, &timestamp, 4);

        // 填充负载
        memcpy(packet.data() + headerSize, data + offset, currentPayloadSize);

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
    }

    return true;
}

void UdpSender::stop() {
    running = false;

    if (sock != INVALID_SOCKET) {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
}
