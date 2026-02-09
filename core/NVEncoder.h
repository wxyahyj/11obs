#pragma once

#include <vector>
#include <stdint.h>
#include <string>

using namespace std;

// 前向声明，避免直接依赖DirectX头文件
class ID3D11Device;
class ID3D11DeviceContext;
class ID3D11Texture2D;

// NVIDIA Video Codec SDK头文件
// 注意：需要安装NVIDIA Video Codec SDK并配置正确的包含路径
#ifdef NVENC_AVAILABLE
    #include <nvEncodeAPI.h>
#endif

class NVEncoder {
public:
    NVEncoder();
    ~NVEncoder();

    bool initialize(
        void* device,
        int width,
        int height,
        int fps,
        int bitrateKbps
    );

    void cleanup();

    bool encode(
        void* inputTexture,
        std::vector<uint8_t>& output
    );

    bool isInitialized() const { return initialized; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    std::string getLastError() const { return lastError; }

private:
    bool createEncoderSession();
    bool initializeEncoder();
    bool createInputResource();
    bool createBitstreamBuffer();

private:
    void* d3d11Device = nullptr;
    void* d3d11Context = nullptr;

    int width = 0;
    int height = 0;
    int fps = 0;
    int bitrate = 0;

    bool initialized = false;

    // NVENC相关
#ifdef NVENC_AVAILABLE
    void* nvencEncoder = nullptr;
    void* nvencInputResource = nullptr;
    void* nvencMappedResource = nullptr;
    void* nvencBitstreamBuffer = nullptr;

    // 编码参数
    void* initParams = nullptr;
    void* encodeConfig = nullptr;
#else
    void* nvencEncoder = nullptr;
    void* nvencInputResource = nullptr;
    void* nvencMappedResource = nullptr;
    void* nvencBitstreamBuffer = nullptr;

    // 编码参数
    void* encodeConfig = nullptr;
    void* initParams = nullptr;
#endif

    // 帧计数
    int frameCount = 0;
    
    // 错误信息
    std::string lastError;
};
