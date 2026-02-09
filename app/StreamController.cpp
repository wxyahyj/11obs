#include "StreamController.h"
#include <iostream>
#include <chrono>

StreamController::StreamController()
    : running(false) {
}

StreamController::~StreamController() {
    stop();
}

bool StreamController::start(const StreamConfig& cfg) {
    if (running) {
        std::cerr << "Stream already running" << std::endl;
        return false;
    }

    config = cfg;

    // 初始化屏幕捕获
    if (!screenCapture.initialize(config.width, config.height)) {
        std::cerr << "Failed to initialize screen capture" << std::endl;
        return false;
    }

    // 初始化编码器
    if (!encoder.initialize(
        screenCapture.getDevice(),
        config.width,
        config.height,
        config.fps,
        config.bitrateKbps
    )) {
        std::cerr << "Failed to initialize encoder" << std::endl;
        return false;
    }

    // 初始化UDP发送器
    if (!udpSender.initialize(config.targetIp, config.port)) {
        std::cerr << "Failed to initialize UDP sender" << std::endl;
        return false;
    }

    // 启动线程
    running = true;
    captureThread = std::thread(&StreamController::captureThreadFunc, this);
    encodeThread = std::thread(&StreamController::encodeThreadFunc, this);
    sendThread = std::thread(&StreamController::sendThreadFunc, this);

    // 设置线程优先级
    SetThreadPriority(captureThread.native_handle(), THREAD_PRIORITY_HIGHEST);
    SetThreadPriority(encodeThread.native_handle(), THREAD_PRIORITY_HIGHEST);
    SetThreadPriority(sendThread.native_handle(), THREAD_PRIORITY_ABOVE_NORMAL);

    // 初始化FPS计算
    lastFPSTime = std::chrono::steady_clock::now();
    captureFrameCount = 0;
    encodeFrameCount = 0;
    sendFrameCount = 0;

    std::cout << "Stream started successfully" << std::endl;
    return true;
}

void StreamController::stop() {
    if (!running) {
        return;
    }

    running = false;

    // 通知所有线程
    captureCV.notify_all();
    encodeCV.notify_all();

    // 等待线程结束
    if (captureThread.joinable()) {
        captureThread.join();
    }
    if (encodeThread.joinable()) {
        encodeThread.join();
    }
    if (sendThread.joinable()) {
        sendThread.join();
    }

    // 清理资源
    screenCapture.cleanup();
    encoder.cleanup();
    udpSender.cleanup();

    // 清空队列
    {
        std::lock_guard<std::mutex> lock(captureMutex);
        while (!captureQueue.empty()) {
            captureQueue.pop();
        }
    }
    {
        std::lock_guard<std::mutex> lock(encodeMutex);
        while (!encodeQueue.empty()) {
            encodeQueue.pop();
        }
    }

    std::cout << "Stream stopped" << std::endl;
}

void StreamController::captureThreadFunc() {
    std::cout << "Capture thread started" << std::endl;

    while (running) {
        ScreenCapture::CaptureFrame frame;
        if (screenCapture.captureFrame(frame)) {
            // 检查队列大小，避免缓冲过多
            std::lock_guard<std::mutex> lock(captureMutex);
            if (captureQueue.size() >= static_cast<size_t>(config.captureQueueSize)) {
                // 丢弃旧帧，保持实时性
                captureQueue.pop();
            }
            captureQueue.push(frame);
            captureCV.notify_one();
            captureFrameCount++;
        }

        // 控制采集频率
        std::this_thread::sleep_for(std::chrono::microseconds(1000000 / config.fps));
    }

    std::cout << "Capture thread stopped" << std::endl;
}

void StreamController::encodeThreadFunc() {
    std::cout << "Encode thread started" << std::endl;

    while (running) {
        ScreenCapture::CaptureFrame frame;
        {
            std::unique_lock<std::mutex> lock(captureMutex);
            captureCV.wait(lock, [this] {
                return !captureQueue.empty() || !running;
            });

            if (!running) {
                break;
            }

            if (!captureQueue.empty()) {
                frame = captureQueue.front();
                captureQueue.pop();
            } else {
                continue;
            }
        }

        // 编码帧
        std::vector<uint8_t> encodedData;
        if (encoder.encode(frame.texture.Get(), encodedData)) {
            // 检查队列大小，避免缓冲过多
            std::lock_guard<std::mutex> lock(encodeMutex);
            if (encodeQueue.size() >= static_cast<size_t>(config.encodeQueueSize)) {
                // 丢弃旧帧，保持实时性
                encodeQueue.pop();
            }
            encodeQueue.push(encodedData);
            encodeCV.notify_one();
            encodeFrameCount++;
        }

        // 短暂睡眠，避免CPU占用过高
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    std::cout << "Encode thread stopped" << std::endl;
}

void StreamController::sendThreadFunc() {
    std::cout << "Send thread started" << std::endl;

    while (running) {
        std::vector<uint8_t> encodedData;
        {
            std::unique_lock<std::mutex> lock(encodeMutex);
            encodeCV.wait(lock, [this] {
                return !encodeQueue.empty() || !running;
            });

            if (!running) {
                break;
            }

            if (!encodeQueue.empty()) {
                encodedData = encodeQueue.front();
                encodeQueue.pop();
            } else {
                continue;
            }
        }

        // 发送数据
        if (udpSender.sendFrame(encodedData)) {
            sendFrameCount++;
        }

        // 短暂睡眠，避免CPU占用过高
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    std::cout << "Send thread stopped" << std::endl;
}

void StreamController::updateStats() {
    calculateFPS();
    bytesSent = udpSender.getBytesSent();
    packetsSent = udpSender.getPacketsSent();
}

void StreamController::calculateFPS() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - lastFPSTime
    ).count();

    if (elapsed >= 1000) {
        captureFPS = static_cast<int>(
            captureFrameCount * 1000.0 / elapsed
        );
        encodeFPS = static_cast<int>(
            encodeFrameCount * 1000.0 / elapsed
        );
        sendFPS = static_cast<int>(
            sendFrameCount * 1000.0 / elapsed
        );

        // 重置计数器
        captureFrameCount = 0;
        encodeFrameCount = 0;
        sendFrameCount = 0;
        lastFPSTime = now;
    }
}
