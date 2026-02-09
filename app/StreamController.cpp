#include "StreamController.h"
#include "../core/ScreenCapture.h"
#include "../core/NVEncoder.h"
#include "../core/UdpSender.h"
#include <iostream>
#include <chrono>
#include <thread>

bool StreamController::start(const StreamConfig& cfg) {
    if (running) return false;
    running = true;
    worker = std::thread(&StreamController::streamThread, this, cfg);
    return true;
}

void StreamController::stop() {
    running = false;
    if (worker.joinable())
        worker.join();
}

bool StreamController::isRunning() const {
    return running;
}

void StreamController::streamThread(StreamConfig cfg) {
    std::cout << "Starting stream thread..." << std::endl;
    std::cout << "  Target IP: " << cfg.targetIp << std::endl;
    std::cout << "  Port: " << cfg.port << std::endl;
    std::cout << "  Resolution: " << cfg.width << "x" << cfg.height << std::endl;
    std::cout << "  FPS: " << cfg.fps << std::endl;
    std::cout << "  Bitrate: " << cfg.bitrateKbps << " kbps" << std::endl;

    // 初始化屏幕采集
    ScreenCapture capture(cfg.width, cfg.height);
    if (!capture.initialize()) {
        std::cerr << "Failed to initialize screen capture" << std::endl;
        running = false;
        return;
    }

    // 初始化编码器
    NVEncoder encoder(
        capture.device(),
        cfg.width,
        cfg.height,
        cfg.fps,
        cfg.bitrateKbps
    );

    // 初始化UDP发送器
    UdpSender sender(cfg.targetIp, cfg.port);
    if (!sender.initialize()) {
        std::cerr << "Failed to initialize UDP sender" << std::endl;
        running = false;
        return;
    }

    std::cout << "Stream initialized successfully. Starting to stream..." << std::endl;

    // 控制采集频率
    const auto frameInterval = std::chrono::microseconds(1000000 / cfg.fps);

    while (running) {
        auto start = std::chrono::high_resolution_clock::now();

        // 采集帧
        ID3D11Texture2D* frame = capture.captureFrame();
        if (!frame) {
            // 短暂睡眠，避免CPU占用过高
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            continue;
        }

        // 编码帧
        std::vector<uint8_t> h264;
        if (encoder.encode(frame, h264)) {
            // 发送H.264数据
            sender.send(h264.data(), h264.size());
        }

        // 控制采集频率
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        if (elapsed < frameInterval) {
            std::this_thread::sleep_for(frameInterval - elapsed);
        }
    }

    std::cout << "Stream thread stopped." << std::endl;
}
