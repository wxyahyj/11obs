#pragma once

#include <string>

using namespace std;

class ConfigManager {
public:
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
    };
    
private:
    Config config;
    
public:
    ConfigManager();
    
    // 从命令行参数加载配置
    bool loadFromCommandLine(int argc, char* argv[]);
    
    // 获取配置
    const Config& getConfig() const { return config; }
    
    // 设置默认配置
    void setDefaultConfig();
};