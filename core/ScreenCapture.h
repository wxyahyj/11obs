#pragma once

#include <cstdint>
#include <memory>
#include <vector>

// 前向声明，避免直接依赖DirectX头文件
class ID3D11Device;
class ID3D11DeviceContext;
class ID3D11Texture2D;
class IDXGIOutputDuplication;
class IDXGIOutput;
class IDXGIResource;

class ScreenCapture {
public:
    struct CaptureFrame {
        void* texture; // 简化为void*，避免DirectX依赖
        void* resource; // 简化为void*，避免DirectX依赖
        uint64_t timestamp;
        int frameIndex;
    };

    ScreenCapture();
    ~ScreenCapture();

    bool initialize(int outputWidth, int outputHeight);
    void cleanup();
    bool captureFrame(CaptureFrame& frame);
    void releaseFrame(const CaptureFrame& frame);

    void* getDevice() const { return d3d11Device; }
    int getWidth() const { return outputWidth; }
    int getHeight() const { return outputHeight; }

private:
    bool createD3DDevice();
    bool setupDesktopDuplication();
    bool createOutputTexture();

private:
    // 简化为void*，避免DirectX依赖
    void* d3d11Device = nullptr;
    void* d3d11Context = nullptr;
    void* duplication = nullptr;
    void* dxgiOutput = nullptr;
    void* outputTexture = nullptr;

    // 输出尺寸
    int outputWidth = 0;
    int outputHeight = 0;

    // 原始屏幕尺寸
    int screenWidth = 0;
    int screenHeight = 0;

    // 裁剪区域（中心）
    int cropX = 0;
    int cropY = 0;

    // 帧计数
    int frameCount = 0;
};
