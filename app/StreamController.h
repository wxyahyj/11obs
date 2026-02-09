#pragma once

#include <stdint.h>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>

#include "StreamConfig.h"

// 前向声明
class ScreenCapture;
class NVEncoder;
class UdpSender;

// 避免循环包含的类型定义
struct CaptureFrame {
    void* texture;
    uint64_t timestamp;
};

class StreamController {
public:
    StreamController();
    ~StreamController();

    bool start(const StreamConfig& config);
    void stop();

    bool isRunning() const { return running; }

    // 统计信息
    int getCaptureFPS() const { return captureFPS; }
    int getEncodeFPS() const { return encodeFPS; }
    int getSendFPS() const { return sendFPS; }
    int getBytesSent() const { return bytesSent; }
    int getPacketsSent() const { return packetsSent; }

    void updateStats();

private:
    void captureThreadFunc();
    void encodeThreadFunc();
    void sendThreadFunc();

    void calculateFPS();

private:
    // 配置
    StreamConfig config;

    // 模块实例
    ScreenCapture screenCapture;
    NVEncoder encoder;
    UdpSender udpSender;

    // 线程
    std::thread captureThread;
    std::thread encodeThread;
    std::thread sendThread;

    // 控制标志
    std::atomic<bool> running;

    // 帧队列
    std::queue<CaptureFrame> captureQueue;
    std::queue<std::vector<uint8_t>> encodeQueue;

    // 队列同步
    std::mutex captureMutex;
    std::mutex encodeMutex;
    std::condition_variable captureCV;
    std::condition_variable encodeCV;

    // 统计信息
    int captureFPS = 0;
    int encodeFPS = 0;
    int sendFPS = 0;
    int bytesSent = 0;
    int packetsSent = 0;

    // FPS计算
    int captureFrameCount = 0;
    int encodeFrameCount = 0;
    int sendFrameCount = 0;
    std::chrono::steady_clock::time_point lastFPSTime;
};
