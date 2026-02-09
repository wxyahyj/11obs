#pragma once

#include "StreamConfig.h"
#include <atomic>
#include <thread>

class StreamController {
public:
    bool start(const StreamConfig& cfg);
    void stop();
    bool isRunning() const;

private:
    std::atomic<bool> running{false};
    std::thread worker;

    void streamThread(StreamConfig cfg);
};
