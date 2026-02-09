#include "ConfigManager.h"
#include <iostream>
#include <fstream>
#include <sstream>

// 使用nlohmann/json库解析JSON文件
// 注意：需要将nlohmann/json.hpp头文件添加到项目中
#include "nlohmann/json.hpp"

using json = nlohmann::json;

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

bool ConfigManager::loadFromFile(const std::string& filePath) {
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open config file: " << filePath << std::endl;
            return false;
        }
        
        json j;
        file >> j;
        
        // 解析屏幕采集参数
        if (j.contains("capture")) {
            const auto& capture = j["capture"];
            if (capture.contains("displayIndex")) {
                config.displayIndex = capture["displayIndex"];
            }
            if (capture.contains("outputWidth")) {
                config.outputWidth = capture["outputWidth"];
            }
            if (capture.contains("outputHeight")) {
                config.outputHeight = capture["outputHeight"];
            }
        }
        
        // 解析编码参数
        if (j.contains("encode")) {
            const auto& encode = j["encode"];
            if (encode.contains("frameRate")) {
                config.frameRate = encode["frameRate"];
            }
            if (encode.contains("bitrate")) {
                config.bitrate = encode["bitrate"];
            }
        }
        
        // 解析传输参数
        if (j.contains("transmit")) {
            const auto& transmit = j["transmit"];
            if (transmit.contains("serverIP")) {
                config.serverIP = transmit["serverIP"];
            }
            if (transmit.contains("serverPort")) {
                config.serverPort = transmit["serverPort"];
            }
            if (transmit.contains("maxPacketSize")) {
                config.maxPacketSize = transmit["maxPacketSize"];
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse config file: " << e.what() << std::endl;
        return false;
    }
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
