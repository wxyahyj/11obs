#pragma once

#include <string>
#include <vector>

class ConfigManager {
private:
    struct Config {
        // 屏幕采集参数
        UINT displayIndex;
        UINT outputWidth;
        UINT outputHeight;
        
        // 编码参数
        UINT frameRate;
        UINT bitrate;
        
        // 传输参数
        std::string serverIP;
        UINT serverPort;
        UINT maxPacketSize;
    } config;
    
public:
    ConfigManager();
    
    // 从JSON文件加载配置
    bool loadFromFile(const std::string& filePath);
    
    // 从命令行参数加载配置
    bool loadFromCommandLine(int argc, char* argv[]);
    
    // 获取配置
    const Config& getConfig() const { return config; }
    
    // 设置默认配置
    void setDefaultConfig();
};