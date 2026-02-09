#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <vector>
#include <atomic>

class ScreenCapture {
private:
    ID3D11Device* device;
    ID3D11DeviceContext* deviceContext;
    IDXGIOutputDuplication* duplication;
    ID3D11Texture2D* stagingTexture;
    ID3D11Texture2D* outputTexture;
    ID3D11SamplerState* samplerState;
    ID3D11ShaderResourceView* srv;
    ID3D11RenderTargetView* rtv;
    
    UINT screenWidth;
    UINT screenHeight;
    UINT outputWidth;
    UINT outputHeight;
    
    std::atomic<bool> running;
    
public:
    struct CaptureFrame {
        ID3D11Texture2D* texture;
        UINT width;
        UINT height;
        uint64_t timestamp;
    };
    
    ScreenCapture();
    ~ScreenCapture();
    
    bool initialize(UINT displayIndex = 0, UINT outputWidth = 640, UINT outputHeight = 640);
    bool captureFrame(CaptureFrame& frame);
    void stop();
    
    UINT getScreenWidth() const { return screenWidth; }
    UINT getScreenHeight() const { return screenHeight; }
    UINT getOutputWidth() const { return outputWidth; }
    UINT getOutputHeight() const { return outputHeight; }
    
    ID3D11Device* getDevice() const { return device; }
    ID3D11DeviceContext* getDeviceContext() const { return deviceContext; }
};