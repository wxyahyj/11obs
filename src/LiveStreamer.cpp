#include "LiveStreamer.h"
#include <iostream>
#include <chrono>

LiveStreamer::LiveStreamer()
    : running(false) {
    // 默认配置
    config.displayIndex = 0;
    config.outputWidth = 640;
    config.outputHeight = 640;
    config.frameRate = 200;
    config.bitrate = 15000;
    config.serverIP = "127.0.0.1";
    config.serverPort = 5000;
    config.maxPacketSize = 1400;
}

LiveStreamer::~LiveStreamer() {
    stop();
}

bool LiveStreamer::initialize(const Config& config) {
    this->config = config;
    
    // 初始化屏幕采集
    if (!screenCapture.initialize(config.displayIndex, config.outputWidth, config.outputHeight)) {
        std::cerr << "Failed to initialize screen capture" << std::endl;
        return false;
    }
    
    // 初始化编码器
    if (!encoder.initialize(
        screenCapture.getDevice(),
        screenCapture.getDeviceContext(),
        config.outputWidth,
        config.outputHeight,
        config.frameRate,
        config.bitrate
    )) {
        std::cerr << "Failed to initialize encoder" << std::endl;
        return false;
    }
    
    // 初始化UDP传输
    if (!transmitter.initialize(config.serverIP, config.serverPort, config.maxPacketSize)) {
        std::cerr << "Failed to initialize UDP transmitter" << std::endl;
        return false;
    }
    
    return true;
}

void LiveStreamer::start() {
    if (running) {
        return;
    }
    
    running = true;
    
    // 启动采集线程
    captureThread = std::thread(&LiveStreamer::captureThreadFunc, this);
    
    // 启动编码线程
    encodeThread = std::thread(&LiveStreamer::encodeThreadFunc, this);
    
    // 启动传输线程
    transmitThread = std::thread(&LiveStreamer::transmitThreadFunc, this);
    
    // 设置线程优先级
    SetThreadPriority(captureThread.native_handle(), THREAD_PRIORITY_HIGHEST);
    SetThreadPriority(encodeThread.native_handle(), THREAD_PRIORITY_HIGHEST);
    SetThreadPriority(transmitThread.native_handle(), THREAD_PRIORITY_ABOVE_NORMAL);
}

void LiveStreamer::stop() {
    if (!running) {
        return;
    }
    
    running = false;
    
    // 等待线程结束
    if (captureThread.joinable()) {
        captureThread.join();
    }
    if (encodeThread.joinable()) {
        encodeThread.join();
    }
    if (transmitThread.joinable()) {
        transmitThread.join();
    }
    
    // 停止模块
    screenCapture.stop();
    encoder.stop();
    transmitter.stop();
}

void LiveStreamer::captureThreadFunc() {
    while (running) {
        ScreenCapture::CaptureFrame frame;
        if (screenCapture.captureFrame(frame)) {
            // 检查队列大小，避免缓冲过多
            if (!captureQueue.empty()) {
                // 丢弃旧帧，保持实时性
                ScreenCapture::CaptureFrame oldFrame;
                captureQueue.pop(oldFrame);
            }
            captureQueue.push(frame);
        }
        
        // 控制采集频率
        std::this_thread::sleep_for(std::chrono::microseconds(1000000 / config.frameRate));
    }
}

void LiveStreamer::encodeThreadFunc() {
    while (running) {
        ScreenCapture::CaptureFrame captureFrame;
        if (captureQueue.pop(captureFrame)) {
            NVEncoder::EncodedFrame encodeFrame;
            if (encoder.encodeFrame(captureFrame.texture, encodeFrame)) {
                // 检查队列大小，避免缓冲过多
                if (!encodeQueue.empty()) {
                    // 丢弃旧帧，保持实时性
                    NVEncoder::EncodedFrame oldFrame;
                    encodeQueue.pop(oldFrame);
                }
                encodeQueue.push(encodeFrame);
            }
        }
        
        // 短暂睡眠，避免CPU占用过高
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

void LiveStreamer::transmitThreadFunc() {
    while (running) {
        NVEncoder::EncodedFrame encodeFrame;
        if (encodeQueue.pop(encodeFrame)) {
            UDPTransmitter::UDPFrame udpFrame;
            udpFrame.data = std::move(encodeFrame.data);
            udpFrame.frameId = encodeFrame.frameId;
            udpFrame.timestamp = encodeFrame.timestamp;
            
            transmitter.sendFrame(udpFrame);
        }
        
        // 短暂睡眠，避免CPU占用过高
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}
