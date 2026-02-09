#include "LiveStreamer.h"
#include "ConfigManager.h"
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
    std::cout << "Low Latency Live Streamer for Windows" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    // 初始化配置管理器
    ConfigManager configManager;
    
    // 加载默认配置
    configManager.setDefaultConfig();
    
    // 尝试从命令行参数加载配置
    if (!configManager.loadFromCommandLine(argc, argv)) {
        std::cerr << "Warning: Failed to load config from command line, using default config" << std::endl;
    }
    
    // 注意：配置文件加载功能已暂时移除，仅支持命令行参数配置
    // std::string configFile = "config.json";
    // if (!configManager.loadFromFile(configFile)) {
    //     std::cerr << "Warning: Failed to load config from file, using default config" << std::endl;
    // }
    
    // 获取配置
    const auto& config = configManager.getConfig();
    
    // 显示配置信息
    std::cout << "Configuration:" << std::endl;
    std::cout << "  Display Index: " << config.displayIndex << std::endl;
    std::cout << "  Output Resolution: " << config.outputWidth << "x" << config.outputHeight << std::endl;
    std::cout << "  Frame Rate: " << config.frameRate << " FPS" << std::endl;
    std::cout << "  Bitrate: " << config.bitrate << " kbps" << std::endl;
    std::cout << "  Server IP: " << config.serverIP << std::endl;
    std::cout << "  Server Port: " << config.serverPort << std::endl;
    std::cout << "  Max Packet Size: " << config.maxPacketSize << " bytes" << std::endl;
    
    // 初始化LiveStreamer
    LiveStreamer streamer;
    
    // 配置LiveStreamer
    LiveStreamer::Config streamerConfig;
    streamerConfig.displayIndex = config.displayIndex;
    streamerConfig.outputWidth = config.outputWidth;
    streamerConfig.outputHeight = config.outputHeight;
    streamerConfig.frameRate = config.frameRate;
    streamerConfig.bitrate = config.bitrate;
    streamerConfig.serverIP = config.serverIP;
    streamerConfig.serverPort = config.serverPort;
    streamerConfig.maxPacketSize = config.maxPacketSize;
    
    // 初始化
    if (!streamer.initialize(streamerConfig)) {
        std::cerr << "Failed to initialize LiveStreamer" << std::endl;
        return 1;
    }
    
    std::cout << "LiveStreamer initialized successfully" << std::endl;
    std::cout << "Press Ctrl+C to stop..." << std::endl;
    
    // 开始推流
    streamer.start();
    
    // 等待用户输入
    std::string input;
    std::getline(std::cin, input);
    
    // 停止推流
    std::cout << "Stopping LiveStreamer..." << std::endl;
    streamer.stop();
    
    std::cout << "LiveStreamer stopped" << std::endl;
    
    return 0;
}