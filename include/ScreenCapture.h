#pragma once

#include <cstdint>
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
    
    unsigned int screenWidth;
    unsigned int screenHeight;
    unsigned int outputWidth;
    unsigned int outputHeight;
    
    std::atomic<bool> running;
    
public:
    struct CaptureFrame {
        ID3D11Texture2D* texture;
        unsigned int width;
        unsigned int height;
        uint64_t timestamp;
    };
    
    ScreenCapture();
    ~ScreenCapture();
    
    bool initialize(unsigned int displayIndex = 0, unsigned int outputWidth = 640, unsigned int outputHeight = 640);
    bool captureFrame(CaptureFrame& frame);
    void stop();
    
    unsigned int getScreenWidth() const { return screenWidth; }
    unsigned int getScreenHeight() const { return screenHeight; }
    unsigned int getOutputWidth() const { return outputWidth; }
    unsigned int getOutputHeight() const { return outputHeight; }
    
    ID3D11Device* getDevice() const { return device; }
    ID3D11DeviceContext* getDeviceContext() const { return deviceContext; }
};