#include "ScreenCapture.h"
#include <iostream>
#include <dxgi.h>

ScreenCapture::ScreenCapture(int width, int height)
    : d3dDevice(nullptr),
      d3dContext(nullptr),
      duplication(nullptr),
      outputTexture(nullptr),
      width(width),
      height(height),
      running(false) {
}

ScreenCapture::~ScreenCapture() {
    stop();
}

bool ScreenCapture::initialize() {
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
        &d3dDevice,
        &featureLevel,
        &d3dContext
    );

    if (FAILED(hr)) {
        std::cerr << "Failed to create D3D11 device: " << hr << std::endl;
        return false;
    }

    // 获取DXGI设备和适配器
    IDXGIDevice* dxgiDevice = nullptr;
    hr = d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
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

    // 获取显示输出
    IDXGIOutput* dxgiOutput = nullptr;
    hr = dxgiAdapter->EnumOutputs(0, &dxgiOutput);
    dxgiAdapter->Release();
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

    // 创建DXGI复制接口
    IDXGIOutput1* dxgiOutput1 = nullptr;
    hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), (void**)&dxgiOutput1);
    dxgiOutput->Release();
    if (FAILED(hr)) {
        std::cerr << "Failed to get DXGI output1: " << hr << std::endl;
        return false;
    }

    hr = dxgiOutput1->DuplicateOutput(d3dDevice, &duplication);
    dxgiOutput1->Release();
    if (FAILED(hr)) {
        std::cerr << "Failed to create output duplication: " << hr << std::endl;
        return false;
    }

    // 创建输出纹理
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

    hr = d3dDevice->CreateTexture2D(&textureDesc, nullptr, &outputTexture);
    if (FAILED(hr)) {
        std::cerr << "Failed to create output texture: " << hr << std::endl;
        return false;
    }

    running = true;
    return true;
}

ID3D11Texture2D* ScreenCapture::captureFrame() {
    if (!running || !duplication) {
        return nullptr;
    }

    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    IDXGIResource* desktopResource = nullptr;

    HRESULT hr = duplication->AcquireNextFrame(0, &frameInfo, &desktopResource);
    if (hr == DXGI_ERROR_ACCESS_LOST) {
        std::cerr << "Display adapter lost, need to reinitialize" << std::endl;
        return nullptr;
    }
    else if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
        return nullptr;
    }
    else if (FAILED(hr)) {
        std::cerr << "Failed to acquire next frame: " << hr << std::endl;
        return nullptr;
    }

    // 获取桌面纹理
    ID3D11Texture2D* desktopTexture = nullptr;
    hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&desktopTexture);
    desktopResource->Release();
    if (FAILED(hr)) {
        duplication->ReleaseFrame();
        std::cerr << "Failed to get desktop texture: " << hr << std::endl;
        return nullptr;
    }

    // 获取桌面纹理描述
    D3D11_TEXTURE2D_DESC desktopDesc;
    desktopTexture->GetDesc(&desktopDesc);

    // 计算裁剪区域（中心区域）
    int desktopWidth = desktopDesc.Width;
    int desktopHeight = desktopDesc.Height;
    int cropX = (desktopWidth - width) / 2;
    int cropY = (desktopHeight - height) / 2;

    // 设置视口
    D3D11_VIEWPORT viewport = {};
    viewport.Width = (float)width;
    viewport.Height = (float)height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    d3dContext->RSSetViewports(1, &viewport);

    // 创建渲染目标视图
    ID3D11RenderTargetView* rtv = nullptr;
    HRESULT rtvHr = d3dDevice->CreateRenderTargetView(outputTexture, nullptr, &rtv);
    if (SUCCEEDED(rtvHr)) {
        // 清除渲染目标
        float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        d3dContext->ClearRenderTargetView(rtv, clearColor);

        // 创建着色器资源视图
        ID3D11ShaderResourceView* srv = nullptr;
        HRESULT srvHr = d3dDevice->CreateShaderResourceView(desktopTexture, nullptr, &srv);
        if (SUCCEEDED(srvHr)) {
            // 创建采样器状态
            D3D11_SAMPLER_DESC samplerDesc = {};
            samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
            samplerDesc.MinLOD = 0;
            samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

            ID3D11SamplerState* samplerState = nullptr;
            HRESULT samplerHr = d3dDevice->CreateSamplerState(&samplerDesc, &samplerState);
            if (SUCCEEDED(samplerHr)) {
                // 设置着色器资源和采样器
                d3dContext->PSSetShaderResources(0, 1, &srv);
                d3dContext->PSSetSamplers(0, 1, &samplerState);

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
                HRESULT bufferHr = d3dDevice->CreateBuffer(&bufferDesc, &initData, &vertexBuffer);
                if (SUCCEEDED(bufferHr)) {
                    UINT stride = 2 * sizeof(float);
                    UINT offset = 0;
                    d3dContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
                    d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

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
                            shaderHr = d3dDevice->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, &vertexShader);

                            // 创建输入布局
                            D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
                                { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
                            };

                            d3dDevice->CreateInputLayout(inputDesc, 1, vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &inputLayout);
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
                            d3dDevice->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, &pixelShader);
                            pixelShaderBlob->Release();
                        }
                    }

                    // 设置着色器
                    d3dContext->VSSetShader(vertexShader, nullptr, 0);
                    d3dContext->PSSetShader(pixelShader, nullptr, 0);
                    d3dContext->IASetInputLayout(inputLayout);

                    // 绘制
                    d3dContext->OMSetRenderTargets(1, &rtv, nullptr);
                    d3dContext->Draw(4, 0);

                    vertexBuffer->Release();
                }

                samplerState->Release();
            }

            srv->Release();
        }

        rtv->Release();
    }

    desktopTexture->Release();
    duplication->ReleaseFrame();

    return outputTexture;
}

void ScreenCapture::stop() {
    running = false;

    if (outputTexture) {
        outputTexture->Release();
        outputTexture = nullptr;
    }

    if (duplication) {
        duplication->Release();
        duplication = nullptr;
    }

    if (d3dContext) {
        d3dContext->Release();
        d3dContext = nullptr;
    }

    if (d3dDevice) {
        d3dDevice->Release();
        d3dDevice = nullptr;
    }
}
