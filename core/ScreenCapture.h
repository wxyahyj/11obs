#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <cstdint>
#include <memory>

using Microsoft::WRL::ComPtr;

class ScreenCapture {
public:
    struct CaptureFrame {
        ComPtr<ID3D11Texture2D> texture;
        ComPtr<IDXGIResource> resource;
        uint64_t timestamp;
        int frameIndex;
    };

    ScreenCapture();
    ~ScreenCapture();

    bool initialize(int outputWidth, int outputHeight);
    void cleanup();
    bool captureFrame(CaptureFrame& frame);
    void releaseFrame(const CaptureFrame& frame);

    ID3D11Device* getDevice() const { return d3d11Device.Get(); }
    ID3D11DeviceContext* getContext() const { return d3d11Context.Get(); }
    int getWidth() const { return outputWidth; }
    int getHeight() const { return outputHeight; }

private:
    bool createD3DDevice();
    bool setupDesktopDuplication();
    bool createOutputTexture();

private:
    // D3D11设备和上下文
    ComPtr<ID3D11Device> d3d11Device;
    ComPtr<ID3D11DeviceContext> d3d11Context;

    // DXGI输出和复制接口
    ComPtr<IDXGIOutputDuplication> duplication;
    ComPtr<IDXGIOutput> dxgiOutput;

    // 输出纹理（裁剪后的中心区域）
    ComPtr<ID3D11Texture2D> outputTexture;

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
