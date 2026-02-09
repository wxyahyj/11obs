#include "ScreenCapture.h"
#include <iostream>
#include <chrono>
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

ScreenCapture::ScreenCapture()
    : device(nullptr),
      deviceContext(nullptr),
      duplication(nullptr),
      stagingTexture(nullptr),
      outputTexture(nullptr),
      samplerState(nullptr),
      srv(nullptr),
      rtv(nullptr),
      screenWidth(0),
      screenHeight(0),
      outputWidth(0),
      outputHeight(0),
      running(false) {
}

ScreenCapture::~ScreenCapture() {
    stop();
    
    if (rtv) rtv->Release();
    if (srv) srv->Release();
    if (samplerState) samplerState->Release();
    if (outputTexture) outputTexture->Release();
    if (stagingTexture) stagingTexture->Release();
    if (duplication) duplication->Release();
    if (deviceContext) deviceContext->Release();
    if (device) device->Release();
}

bool ScreenCapture::initialize(UINT displayIndex, UINT outWidth, UINT outHeight) {
    outputWidth = outWidth;
    outputHeight = outHeight;
    
    // 创建D3D11设备
    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &device,
        &featureLevel,
        &deviceContext
    );
    
    if (FAILED(hr)) {
        std::cerr << "Failed to create D3D11 device: " << hr << std::endl;
        return false;
    }
    
    // 获取DXGI工厂
    IDXGIDevice* dxgiDevice = nullptr;
    hr = device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    if (FAILED(hr)) {
        std::cerr << "Failed to get DXGI device: " << hr << std::endl;
        return false;
    }
    
    IDXGIAdapter* dxgiAdapter = nullptr;
    hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
    dxgiDevice->Release();
    if (FAILED(hr)) {
        std::cerr << "Failed to get DXGI adapter: " << hr << std::endl;
        return false;
    }
    
    IDXGIFactory1* dxgiFactory = nullptr;
    hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory1), (void**)&dxgiFactory);
    dxgiAdapter->Release();
    if (FAILED(hr)) {
        std::cerr << "Failed to get DXGI factory: " << hr << std::endl;
        return false;
    }
    
    // 获取显示输出
    IDXGIOutput* dxgiOutput = nullptr;
    hr = dxgiFactory->EnumOutputs(displayIndex, &dxgiOutput);
    dxgiFactory->Release();
    if (FAILED(hr)) {
        std::cerr << "Failed to get DXGI output: " << hr << std::endl;
        return false;
    }
    
    // 获取显示模式
    DXGI_OUTPUT_DESC outputDesc;
    hr = dxgiOutput->GetDesc(&outputDesc);
    if (FAILED(hr)) {
        std::cerr << "Failed to get output desc: " << hr << std::endl;
        dxgiOutput->Release();
        return false;
    }
    
    screenWidth = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
    screenHeight = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;
    
    // 创建DXGI复制接口
    IDXGIOutput1* dxgiOutput1 = nullptr;
    hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), (void**)&dxgiOutput1);
    dxgiOutput->Release();
    if (FAILED(hr)) {
        std::cerr << "Failed to get DXGI output1: " << hr << std::endl;
        return false;
    }
    
    hr = dxgiOutput1->DuplicateOutput(device, &duplication);
    dxgiOutput1->Release();
    if (FAILED(hr)) {
        std::cerr << "Failed to create output duplication: " << hr << std::endl;
        return false;
    }
    
    // 创建输出纹理
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = outputWidth;
    textureDesc.Height = outputHeight;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    
    hr = device->CreateTexture2D(&textureDesc, nullptr, &outputTexture);
    if (FAILED(hr)) {
        std::cerr << "Failed to create output texture: " << hr << std::endl;
        return false;
    }
    
    // 创建着色器资源视图
    hr = device->CreateShaderResourceView(outputTexture, nullptr, &srv);
    if (FAILED(hr)) {
        std::cerr << "Failed to create SRV: " << hr << std::endl;
        return false;
    }
    
    // 创建渲染目标视图
    hr = device->CreateRenderTargetView(outputTexture, nullptr, &rtv);
    if (FAILED(hr)) {
        std::cerr << "Failed to create RTV: " << hr << std::endl;
        return false;
    }
    
    // 创建采样器状态
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    
    hr = device->CreateSamplerState(&samplerDesc, &samplerState);
    if (FAILED(hr)) {
        std::cerr << "Failed to create sampler state: " << hr << std::endl;
        return false;
    }
    
    running = true;
    return true;
}

bool ScreenCapture::captureFrame(CaptureFrame& frame) {
    if (!running || !duplication) {
        return false;
    }
    
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    IDXGIResource* desktopResource = nullptr;
    
    HRESULT hr = duplication->AcquireNextFrame(0, &frameInfo, &desktopResource);
    if (hr == DXGI_ERROR_ACCESS_LOST) {
        std::cerr << "Display adapter lost, need to reinitialize" << std::endl;
        return false;
    }
    else if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
        return false;
    }
    else if (FAILED(hr)) {
        std::cerr << "Failed to acquire next frame: " << hr << std::endl;
        return false;
    }
    
    // 获取桌面纹理
    ID3D11Texture2D* desktopTexture = nullptr;
    hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&desktopTexture);
    desktopResource->Release();
    if (FAILED(hr)) {
        duplication->ReleaseFrame();
        std::cerr << "Failed to get desktop texture: " << hr << std::endl;
        return false;
    }
    
    // 获取桌面纹理描述
    D3D11_TEXTURE2D_DESC desktopDesc;
    desktopTexture->GetDesc(&desktopDesc);
    
    // 计算裁剪区域
    int cropX = (desktopDesc.Width - outputWidth) / 2;
    int cropY = (desktopDesc.Height - outputHeight) / 2;
    
    // 设置视口
    D3D11_VIEWPORT viewport = {};
    viewport.Width = (float)outputWidth;
    viewport.Height = (float)outputHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewport);
    
    // 清除渲染目标
    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    deviceContext->ClearRenderTargetView(rtv, clearColor);
    
    // 创建临时着色器资源视图
    ID3D11ShaderResourceView* desktopSRV = nullptr;
    hr = device->CreateShaderResourceView(desktopTexture, nullptr, &desktopSRV);
    if (SUCCEEDED(hr)) {
        // 设置着色器资源
        deviceContext->PSSetShaderResources(0, 1, &desktopSRV);
        deviceContext->PSSetSamplers(0, 1, &samplerState);
        
        // 绘制全屏四边形进行裁剪和缩放
        float vertices[] = {
            -1.0f, -1.0f,
            1.0f, -1.0f,
            -1.0f, 1.0f,
            1.0f, 1.0f
        };
        
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.ByteWidth = sizeof(vertices);
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        
        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = vertices;
        
        ID3D11Buffer* vertexBuffer = nullptr;
        hr = device->CreateBuffer(&bufferDesc, &initData, &vertexBuffer);
        if (SUCCEEDED(hr)) {
            UINT stride = 2 * sizeof(float);
            UINT offset = 0;
            deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
            deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            
            // 创建和编译着色器
            static ID3D11VertexShader* vertexShader = nullptr;
            static ID3D11PixelShader* pixelShader = nullptr;
            static ID3D11InputLayout* inputLayout = nullptr;
            
            if (!vertexShader || !pixelShader || !inputLayout) {
                // 顶点着色器代码
                const char* vertexShaderCode = "\n"
                    "struct VertexIn {\n"
                    "    float2 pos : POSITION;\n"
                    "};\n"
                    "struct VertexOut {\n"
                    "    float4 pos : SV_POSITION;\n"
                    "    float2 uv : TEXCOORD0;\n"
                    "};\n"
                    "VertexOut VS(VertexIn input) {\n"
                    "    VertexOut output;\n"
                    "    output.pos = float4(input.pos, 0.0f, 1.0f);\n"
                    "    output.uv = float2((input.pos.x + 1.0f) * 0.5f, 1.0f - (input.pos.y + 1.0f) * 0.5f);\n"
                    "    return output;\n"
                    "}\n";
                
                // 像素着色器代码
                const char* pixelShaderCode = "\n"
                    "Texture2D g_Texture : register(t0);\n"
                    "SamplerState g_Sampler : register(s0);\n"
                    "float4 PS(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target {\n"
                    "    return g_Texture.Sample(g_Sampler, uv);\n"
                    "}\n";
                
                // 编译顶点着色器
                ID3DBlob* vertexShaderBlob = nullptr;
                HRESULT shaderHr = D3DCompile(
                    vertexShaderCode,
                    strlen(vertexShaderCode),
                    nullptr,
                    nullptr,
                    nullptr,
                    "VS",
                    "vs_4_0",
                    0,
                    0,
                    &vertexShaderBlob,
                    nullptr
                );
                
                if (SUCCEEDED(shaderHr)) {
                    shaderHr = device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, &vertexShader);
                    
                    // 创建输入布局
                    D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
                        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
                    };
                    
                    device->CreateInputLayout(inputDesc, 1, vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &inputLayout);
                    vertexShaderBlob->Release();
                }
                
                // 编译像素着色器
                ID3DBlob* pixelShaderBlob = nullptr;
                shaderHr = D3DCompile(
                    pixelShaderCode,
                    strlen(pixelShaderCode),
                    nullptr,
                    nullptr,
                    nullptr,
                    "PS",
                    "ps_4_0",
                    0,
                    0,
                    &pixelShaderBlob,
                    nullptr
                );
                
                if (SUCCEEDED(shaderHr)) {
                    device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, &pixelShader);
                    pixelShaderBlob->Release();
                }
            }
            
            // 设置着色器
            deviceContext->VSSetShader(vertexShader, nullptr, 0);
            deviceContext->PSSetShader(pixelShader, nullptr, 0);
            deviceContext->IASetInputLayout(inputLayout);
            
            // 绘制
            deviceContext->Draw(4, 0);
            
            vertexBuffer->Release();
        }
        
        desktopSRV->Release();
    }
    
    desktopTexture->Release();
    duplication->ReleaseFrame();
    
    // 填充帧信息
    frame.texture = outputTexture;
    frame.width = outputWidth;
    frame.height = outputHeight;
    frame.timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()
    ).count();
    
    return true;
}

void ScreenCapture::stop() {
    running = false;
}