#pragma once

#include "ScreenCapture.h"
#include "NVEncoder.h"
#include "UDPTransmitter.h"
#include "LockFreeQueue.h"
#include <thread>
#include <atomic>
#include <string>

class LiveStreamer {
private:
    // 模块实例
    ScreenCapture screenCapture;
    NVEncoder encoder;
    UDPTransmitter transmitter;
    
    // 无锁队列用于线程间通信
    LockFreeQueue<ScreenCapture::CaptureFrame> captureQueue;
    LockFreeQueue<NVEncoder::EncodedFrame> encodeQueue;
    
    // 线程
    std::thread captureThread;
    std::thread encodeThread;
    std::thread transmitThread;
    
    // 控制标志
    std::atomic<bool> running;
    
    // 配置参数
    struct Config {
        // 屏幕采集参数
        unsigned int displayIndex;
        unsigned int outputWidth;
        unsigned int outputHeight;
        
        // 编码参数
        unsigned int frameRate;
        unsigned int bitrate;
        
        // 传输参数
        std::string serverIP;
        unsigned int serverPort;
        unsigned int maxPacketSize;
    } config;
    
    // 线程函数
    void captureThreadFunc();
    void encodeThreadFunc();
    void transmitThreadFunc();
    
public:
    LiveStreamer();
    ~LiveStreamer();
    
    // 初始化
    bool initialize(const Config& config);
    
    // 开始推流
    void start();
    
    // 停止推流
    void stop();
    
    // 获取状态
    bool isRunning() const { return running; }
};