#include "ScreenCapture.h"
#include <iostream>
#include <chrono>

ScreenCapture::ScreenCapture() {
}

ScreenCapture::~ScreenCapture() {
    cleanup();
}

bool ScreenCapture::createD3DDevice() {
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &d3d11Device,
        &featureLevel,
        &d3d11Context
    );

    if (FAILED(hr)) {
        std::cerr << "Failed to create D3D11 device: " << std::hex << hr << std::endl;
        return false;
    }

    std::cout << "D3D11 device created successfully" << std::endl;
    return true;
}

bool ScreenCapture::setupDesktopDuplication() {
    ComPtr<IDXGIDevice> dxgiDevice;
    HRESULT hr = d3d11Device.As(&dxgiDevice);
    if (FAILED(hr)) {
        std::cerr << "Failed to get DXGI device" << std::endl;
        return false;
    }

    ComPtr<IDXGIAdapter> dxgiAdapter;
    hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), &dxgiAdapter);
    if (FAILED(hr)) {
        std::cerr << "Failed to get DXGI adapter" << std::endl;
        return false;
    }

    // 获取第一个输出（主显示器）
    hr = dxgiAdapter->EnumOutputs(0, &dxgiOutput);
    if (FAILED(hr)) {
        std::cerr << "Failed to get DXGI output" << std::endl;
        return false;
    }

    // 获取输出描述
    DXGI_OUTPUT_DESC outputDesc;
    hr = dxgiOutput->GetDesc(&outputDesc);
    if (FAILED(hr)) {
        std::cerr << "Failed to get output description" << std::endl;
        return false;
    }

    screenWidth = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
    screenHeight = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;

    std::cout << "Screen resolution: " << screenWidth << "x" << screenHeight << std::endl;

    // 计算裁剪区域（中心）
    cropX = (screenWidth - outputWidth) / 2;
    cropY = (screenHeight - outputHeight) / 2;

    // 确保裁剪区域在屏幕范围内
    if (cropX < 0) cropX = 0;
    if (cropY < 0) cropY = 0;
    if (cropX + outputWidth > screenWidth) cropX = screenWidth - outputWidth;
    if (cropY + outputHeight > screenHeight) cropY = screenHeight - outputHeight;

    std::cout << "Crop region: (" << cropX << ", " << cropY << ") to (" 
              << (cropX + outputWidth) << ", " << (cropY + outputHeight) << ")" << std::endl;

    // 创建Desktop Duplication
    ComPtr<IDXGIOutput1> output1;
    hr = dxgiOutput.As(&output1);
    if (FAILED(hr)) {
        std::cerr << "Failed to get IDXGIOutput1" << std::endl;
        return false;
    }

    hr = output1->DuplicateOutput(d3d11Device.Get(), &duplication);
    if (FAILED(hr)) {
        std::cerr << "Failed to create desktop duplication: " << std::hex << hr << std::endl;
        return false;
    }

    std::cout << "Desktop duplication created successfully" << std::endl;
    return true;
}

bool ScreenCapture::createOutputTexture() {
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = outputWidth;
    texDesc.Height = outputHeight;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    HRESULT hr = d3d11Device->CreateTexture2D(&texDesc, nullptr, &outputTexture);
    if (FAILED(hr)) {
        std::cerr << "Failed to create output texture" << std::endl;
        return false;
    }

    std::cout << "Output texture created: " << outputWidth << "x" << outputHeight << std::endl;
    return true;
}

bool ScreenCapture::initialize(int w, int h) {
    outputWidth = w;
    outputHeight = h;

    if (!createD3DDevice()) {
        return false;
    }

    if (!setupDesktopDuplication()) {
        return false;
    }

    if (!createOutputTexture()) {
        return false;
    }

    std::cout << "ScreenCapture initialized successfully" << std::endl;
    return true;
}

void ScreenCapture::cleanup() {
    if (duplication) {
        duplication->ReleaseFrame();
    }
    duplication.Reset();
    dxgiOutput.Reset();
    outputTexture.Reset();
    d3d11Context.Reset();
    d3d11Device.Reset();
}

bool ScreenCapture::captureFrame(CaptureFrame& frame) {
    if (!duplication) {
        return false;
    }

    // 获取下一帧
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    ComPtr<IDXGIResource> resource;
    HRESULT hr = duplication->AcquireNextFrame(INFINITE, &frameInfo, &resource);
    
    if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
        return false;
    }

    if (FAILED(hr)) {
        if (hr == DXGI_ERROR_ACCESS_LOST) {
            std::cerr << "Desktop duplication access lost, reinitializing..." << std::endl;
            duplication->ReleaseFrame();
            duplication.Reset();
            setupDesktopDuplication();
        }
        return false;
    }

    // 获取纹理
    ComPtr<ID3D11Texture2D> srcTexture;
    hr = resource.As(&srcTexture);
    if (FAILED(hr)) {
        std::cerr << "Failed to get texture from resource" << std::endl;
        duplication->ReleaseFrame();
        return false;
    }

    // 获取纹理描述
    D3D11_TEXTURE2D_DESC srcDesc;
    srcTexture->GetDesc(&srcDesc);

    // 复制到输出纹理（裁剪中心区域）
    D3D11_BOX srcBox;
    srcBox.left = cropX;
    srcBox.top = cropY;
    srcBox.front = 0;
    srcBox.right = cropX + outputWidth;
    srcBox.bottom = cropY + outputHeight;
    srcBox.back = 1;

    d3d11Context->CopySubresourceRegion(
        outputTexture.Get(),
        0,
        0, 0, 0,
        srcTexture.Get(),
        0,
        &srcBox
    );

    // 释放帧
    duplication->ReleaseFrame();

    // 填充帧信息
    frame.texture = outputTexture;
    frame.resource = resource;
    frame.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    frame.frameIndex = frameCount++;

    return true;
}

void ScreenCapture::releaseFrame(const CaptureFrame& frame) {
    // 帧资源由ComPtr自动管理
}
