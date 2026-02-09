#include "ConfigManager.h"
#include <iostream>

ConfigManager::ConfigManager() {
    setDefaultConfig();
}

void ConfigManager::setDefaultConfig() {
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

bool ConfigManager::loadFromCommandLine(int argc, char* argv[]) {
    try {
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            
            // 解析屏幕采集参数
            if (arg == "--display") {
                if (i + 1 < argc) {
                    config.displayIndex = std::stoi(argv[++i]);
                }
            } else if (arg == "--width") {
                if (i + 1 < argc) {
                    config.outputWidth = std::stoi(argv[++i]);
                }
            } else if (arg == "--height") {
                if (i + 1 < argc) {
                    config.outputHeight = std::stoi(argv[++i]);
                }
            }
            
            // 解析编码参数
            else if (arg == "--fps") {
                if (i + 1 < argc) {
                    config.frameRate = std::stoi(argv[++i]);
                }
            } else if (arg == "--bitrate") {
                if (i + 1 < argc) {
                    config.bitrate = std::stoi(argv[++i]);
                }
            }
            
            // 解析传输参数
            else if (arg == "--server") {
                if (i + 1 < argc) {
                    config.serverIP = argv[++i];
                }
            } else if (arg == "--port") {
                if (i + 1 < argc) {
                    config.serverPort = std::stoi(argv[++i]);
                }
            } else if (arg == "--max-packet-size") {
                if (i + 1 < argc) {
                    config.maxPacketSize = std::stoi(argv[++i]);
                }
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse command line arguments: " << e.what() << std::endl;
        return false;
    }
}
