#pragma once

#include <string>

struct StreamConfig {
    // 网络配置
    char targetIp[64] = "127.0.0.1";
    int port = 4459;

    // 视频配置
    int width = 640;
    int height = 640;
    int fps = 200;
    int bitrateKbps = 15000;

    // 性能配置
    int captureQueueSize = 2;
    int encodeQueueSize = 2;
};
