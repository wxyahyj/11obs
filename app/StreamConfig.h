#pragma once

struct StreamConfig {
    char targetIp[64] = "127.0.0.1";
    int port = 4459;

    int width = 640;
    int height = 640;

    int fps = 200;
    int bitrateKbps = 15000;
};
