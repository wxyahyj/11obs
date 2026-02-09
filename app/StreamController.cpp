#include "StreamController.h"
#include <iostream>
#include <chrono>
#include <stdexcept>

StreamController::StreamController()
    : running(false),
      captureFPS(0),
      encodeFPS(0),
      sendFPS(0),
      bytesSent(0),
      packetsSent(0),
      captureFrameCount(0),
      encodeFrameCount(0),
      sendFrameCount(0)
{
    lastFPSTime = std::chrono::steady_clock::now();
}

StreamController::~StreamController() {
    try {
        stop();
    } catch (const std::exception& e) {
        std::cerr << "Error in StreamController destructor: " << e.what() << std::endl;
    }
}

bool StreamController::start(const StreamConfig& cfg) {
    try {
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
    } catch (const std::exception& e) {
        std::cerr << "Error starting stream: " << e.what() << std::endl;
        stop();
        return false;
    }
}

void StreamController::stop() {
    try {
        if (!running) {
            return;
        }

        std::cout << "Stopping stream..." << std::endl;
        
        // 设置停止标志
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

        std::cout << "Stream stopped successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error stopping stream: " << e.what() << std::endl;
    }
}

void StreamController::captureThreadFunc() {
    try {
        std::cout << "Capture thread started" << std::endl;

        while (running) {
            try {
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
            } catch (const std::exception& e) {
                std::cerr << "Error in capture thread: " << e.what() << std::endl;
                // 短暂暂停后继续
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        std::cout << "Capture thread stopped" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error in capture thread: " << e.what() << std::endl;
        // 确保线程能够退出
        running = false;
    }
}

void StreamController::encodeThreadFunc() {
    try {
        std::cout << "Encode thread started" << std::endl;

        while (running) {
            try {
                ScreenCapture::CaptureFrame frame;
                bool gotFrame = false;
                
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
                        gotFrame = true;
                    }
                }

                if (gotFrame) {
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
                }

                // 短暂睡眠，避免CPU占用过高
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            } catch (const std::exception& e) {
                std::cerr << "Error in encode thread: " << e.what() << std::endl;
                // 短暂暂停后继续
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        std::cout << "Encode thread stopped" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error in encode thread: " << e.what() << std::endl;
        // 确保线程能够退出
        running = false;
    }
}

void StreamController::sendThreadFunc() {
    try {
        std::cout << "Send thread started" << std::endl;

        while (running) {
            try {
                std::vector<uint8_t> encodedData;
                bool gotData = false;
                
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
                        gotData = true;
                    }
                }

                if (gotData) {
                    // 发送数据
                    if (udpSender.sendFrame(encodedData)) {
                        sendFrameCount++;
                    }
                }

                // 短暂睡眠，避免CPU占用过高
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            } catch (const std::exception& e) {
                std::cerr << "Error in send thread: " << e.what() << std::endl;
                // 短暂暂停后继续
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        std::cout << "Send thread stopped" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error in send thread: " << e.what() << std::endl;
        // 确保线程能够退出
        running = false;
    }
}

void StreamController::updateStats() {
    try {
        calculateFPS();
        bytesSent = udpSender.getBytesSent();
        packetsSent = udpSender.getPacketsSent();
    } catch (const std::exception& e) {
        std::cerr << "Error updating stats: " << e.what() << std::endl;
    }
}

void StreamController::calculateFPS() {
    try {
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
    } catch (const std::exception& e) {
        std::cerr << "Error calculating FPS: " << e.what() << std::endl;
    }
}

int StreamController::getCaptureFPS() const {
    return captureFPS;
}

int StreamController::getEncodeFPS() const {
    return encodeFPS;
}

int StreamController::getSendFPS() const {
    return sendFPS;
}

int StreamController::getBytesSent() const {
    return bytesSent;
}

int StreamController::getPacketsSent() const {
    return packetsSent;
}
