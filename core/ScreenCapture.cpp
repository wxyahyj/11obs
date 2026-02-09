#include "ScreenCapture.h"
#include <iostream>
#include <chrono>
#include <stdexcept>

// DirectX头文件
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

ScreenCapture::ScreenCapture()
    : outputWidth(0),
      outputHeight(0),
      screenWidth(0),
      screenHeight(0),
      cropX(0),
      cropY(0),
      frameCount(0)
{
}

ScreenCapture::~ScreenCapture() {
    try {
        cleanup();
    } catch (const std::exception& e) {
        std::cerr << "Error in ScreenCapture destructor: " << e.what() << std::endl;
    }
}

bool ScreenCapture::createD3DDevice() {
    try {
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
        ComPtr<ID3D11Device> device;
        ComPtr<ID3D11DeviceContext> context;
        
        HRESULT hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createDeviceFlags,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            &device,
            &featureLevel,
            &context
        );

        if (FAILED(hr)) {
            std::cerr << "Failed to create D3D11 device: " << std::hex << hr << std::endl;
            return false;
        }

        // 存储为void*
        d3d11Device = device.Detach();
        d3d11Context = context.Detach();

        std::cout << "D3D11 device created successfully" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error creating D3D11 device: " << e.what() << std::endl;
        return false;
    }
}

bool ScreenCapture::setupDesktopDuplication() {
    try {
        if (!d3d11Device) {
            std::cerr << "D3D11 device not initialized" << std::endl;
            return false;
        }

        // 将void*转换回实际类型
        ID3D11Device* device = static_cast<ID3D11Device*>(d3d11Device);
        ComPtr<ID3D11Device> d3dDevice(device);

        ComPtr<IDXGIDevice> dxgiDevice;
        HRESULT hr = d3dDevice.As(&dxgiDevice);
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
        ComPtr<IDXGIOutput> output;
        hr = dxgiAdapter->EnumOutputs(0, &output);
        if (FAILED(hr)) {
            std::cerr << "Failed to get DXGI output" << std::endl;
            return false;
        }
        dxgiOutput = output.Detach();

        // 获取输出描述
        IDXGIOutput* dxgiOut = static_cast<IDXGIOutput*>(dxgiOutput);
        DXGI_OUTPUT_DESC outputDesc;
        hr = dxgiOut->GetDesc(&outputDesc);
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
        hr = output.As(&output1);
        if (FAILED(hr)) {
            std::cerr << "Failed to get IDXGIOutput1" << std::endl;
            return false;
        }

        ComPtr<IDXGIOutputDuplication> dup;
        hr = output1->DuplicateOutput(device, &dup);
        if (FAILED(hr)) {
            std::cerr << "Failed to create desktop duplication: " << std::hex << hr << std::endl;
            return false;
        }
        duplication = dup.Detach();

        std::cout << "Desktop duplication created successfully" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error setting up desktop duplication: " << e.what() << std::endl;
        return false;
    }
}

bool ScreenCapture::createOutputTexture() {
    try {
        if (!d3d11Device) {
            std::cerr << "D3D11 device not initialized" << std::endl;
            return false;
        }

        // 将void*转换回实际类型
        ID3D11Device* device = static_cast<ID3D11Device*>(d3d11Device);

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

        ComPtr<ID3D11Texture2D> texture;
        HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, &texture);
        if (FAILED(hr)) {
            std::cerr << "Failed to create output texture" << std::endl;
            return false;
        }
        outputTexture = texture.Detach();

        std::cout << "Output texture created: " << outputWidth << "x" << outputHeight << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error creating output texture: " << e.what() << std::endl;
        return false;
    }
}

bool ScreenCapture::initialize(int w, int h) {
    try {
        if (w <= 0 || h <= 0) {
            std::cerr << "Invalid output size: " << w << "x" << h << std::endl;
            return false;
        }

        outputWidth = w;
        outputHeight = h;

        if (!createD3DDevice()) {
            std::cerr << "Failed to create D3D11 device" << std::endl;
            return false;
        }

        if (!setupDesktopDuplication()) {
            std::cerr << "Failed to setup desktop duplication" << std::endl;
            cleanup();
            return false;
        }

        if (!createOutputTexture()) {
            std::cerr << "Failed to create output texture" << std::endl;
            cleanup();
            return false;
        }

        std::cout << "ScreenCapture initialized successfully" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error initializing ScreenCapture: " << e.what() << std::endl;
        cleanup();
        return false;
    }
}

void ScreenCapture::cleanup() {
    try {
        if (duplication) {
            // 尝试释放当前帧
            try {
                IDXGIOutputDuplication* dup = static_cast<IDXGIOutputDuplication*>(duplication);
                dup->ReleaseFrame();
            } catch (...) {
                // 忽略可能的异常
            }
        }

        // 释放资源
        if (d3d11Device) {
            ID3D11Device* device = static_cast<ID3D11Device*>(d3d11Device);
            device->Release();
            d3d11Device = nullptr;
        }
        
        if (d3d11Context) {
            ID3D11DeviceContext* context = static_cast<ID3D11DeviceContext*>(d3d11Context);
            context->Release();
            d3d11Context = nullptr;
        }
        
        if (duplication) {
            IDXGIOutputDuplication* dup = static_cast<IDXGIOutputDuplication*>(duplication);
            dup->Release();
            duplication = nullptr;
        }
        
        if (dxgiOutput) {
            IDXGIOutput* output = static_cast<IDXGIOutput*>(dxgiOutput);
            output->Release();
            dxgiOutput = nullptr;
        }
        
        if (outputTexture) {
            ID3D11Texture2D* texture = static_cast<ID3D11Texture2D*>(outputTexture);
            texture->Release();
            outputTexture = nullptr;
        }

        // 重置状态
        outputWidth = 0;
        outputHeight = 0;
        screenWidth = 0;
        screenHeight = 0;
        cropX = 0;
        cropY = 0;
        frameCount = 0;

        std::cout << "ScreenCapture cleaned up" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error cleaning up ScreenCapture: " << e.what() << std::endl;
    }
}

bool ScreenCapture::captureFrame(CaptureFrame& frame) {
    try {
        if (!duplication || !outputTexture) {
            std::cerr << "ScreenCapture not initialized" << std::endl;
            return false;
        }

        // 将void*转换回实际类型
        IDXGIOutputDuplication* dup = static_cast<IDXGIOutputDuplication*>(duplication);
        ID3D11DeviceContext* context = static_cast<ID3D11DeviceContext*>(d3d11Context);
        ID3D11Texture2D* outTexture = static_cast<ID3D11Texture2D*>(outputTexture);

        // 获取下一帧
        DXGI_OUTDUPL_FRAME_INFO frameInfo;
        ComPtr<IDXGIResource> resource;
        HRESULT hr = dup->AcquireNextFrame(1000, &frameInfo, &resource);
        
        if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
            return false;
        }

        if (FAILED(hr)) {
            if (hr == DXGI_ERROR_ACCESS_LOST) {
                std::cerr << "Desktop duplication access lost, reinitializing..." << std::endl;
                try {
                    dup->ReleaseFrame();
                } catch (...) {
                    // 忽略可能的异常
                }
                duplication = nullptr;
                setupDesktopDuplication();
            } else {
                std::cerr << "Failed to acquire next frame: " << std::hex << hr << std::endl;
            }
            return false;
        }

        // 获取纹理
        ComPtr<ID3D11Texture2D> srcTexture;
        hr = resource.As(&srcTexture);
        if (FAILED(hr)) {
            std::cerr << "Failed to get texture from resource" << std::endl;
            dup->ReleaseFrame();
            return false;
        }

        // 复制到输出纹理（裁剪中心区域）
        D3D11_BOX srcBox;
        srcBox.left = cropX;
        srcBox.top = cropY;
        srcBox.front = 0;
        srcBox.right = cropX + outputWidth;
        srcBox.bottom = cropY + outputHeight;
        srcBox.back = 1;

        context->CopySubresourceRegion(
            outTexture,
            0,
            0, 0, 0,
            srcTexture.Get(),
            0,
            &srcBox
        );

        // 释放帧
        dup->ReleaseFrame();

        // 填充帧信息
        frame.texture = outTexture;
        frame.resource = resource.Detach();
        frame.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        frame.frameIndex = frameCount++;

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error capturing frame: " << e.what() << std::endl;
        try {
            if (duplication) {
                IDXGIOutputDuplication* dup = static_cast<IDXGIOutputDuplication*>(duplication);
                dup->ReleaseFrame();
            }
        } catch (...) {
            // 忽略可能的异常
        }
        return false;
    }
}

void ScreenCapture::releaseFrame(const CaptureFrame& frame) {
    // 帧资源由ComPtr自动管理
    // 这里可以添加额外的资源释放逻辑
}
