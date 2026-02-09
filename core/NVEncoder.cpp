#include "NVEncoder.h"
#include <iostream>
#include <cstring>
#include <sstream>

// DirectX头文件
#include <d3d11.h>

NVEncoder::NVEncoder() {
}

NVEncoder::~NVEncoder() {
    cleanup();
}

bool NVEncoder::initialize(
    void* device,
    int w,
    int h,
    int fps_,
    int bitrateKbps
) {
    try {
        if (!device) {
            lastError = "Invalid D3D11 device pointer";
            return false;
        }

        // 将void*转换回实际类型
        ID3D11Device* d3dDevice = static_cast<ID3D11Device*>(device);
        d3d11Device = d3dDevice;
        
        ID3D11DeviceContext* context;
        HRESULT hr = d3dDevice->GetImmediateContext(&context);
        if (FAILED(hr)) {
            std::stringstream ss;
            ss << "Failed to get D3D11 immediate context: " << hr;
            lastError = ss.str();
            return false;
        }
        d3d11Context = context;
        
        width = w;
        height = h;
        fps = fps_;
        bitrate = bitrateKbps;

#ifdef NVENC_AVAILABLE
        if (!createEncoderSession()) {
            return false;
        }

        if (!initializeEncoder()) {
            return false;
        }

        if (!createInputResource()) {
            return false;
        }

        if (!createBitstreamBuffer()) {
            return false;
        }

        initialized = true;
        std::cout << "NVEncoder initialized successfully" << std::endl;
        std::cout << "  Resolution: " << width << "x" << height << std::endl;
        std::cout << "  Frame Rate: " << fps << " FPS" << std::endl;
        std::cout << "  Bitrate: " << bitrate << " kbps" << std::endl;
        return true;
#else
        lastError = "NVENC SDK not available, encoder will not work";
        std::cerr << "NVENC SDK not available, encoder will not work" << std::endl;
        return false;
#endif
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Exception during NVEncoder initialization: " << e.what();
        lastError = ss.str();
        std::cerr << lastError << std::endl;
        cleanup();
        return false;
    }
}

void NVEncoder::cleanup() {
    try {
#ifdef NVENC_AVAILABLE
        if (nvencEncoder) {
            try {
                if (nvencBitstreamBuffer) {
                    nvencEncoder->nvEncDestroyBitstreamBuffer(nvencEncoder, nvencBitstreamBuffer);
                    nvencBitstreamBuffer = nullptr;
                }

                if (nvencMappedResource) {
                    nvencEncoder->nvEncUnmapInputResource(nvencEncoder, nvencMappedResource);
                    nvencMappedResource = nullptr;
                }

                if (nvencInputResource) {
                    nvencEncoder->nvEncUnregisterResource(nvencEncoder, nvencInputResource);
                    nvencInputResource = nullptr;
                }

                nvencEncoder->nvEncDestroyEncoder(nvencEncoder);
            } catch (...) {
                // 忽略清理时的异常
            }
            delete nvencEncoder;
            nvencEncoder = nullptr;
        }

        if (initParams) {
            delete initParams;
            initParams = nullptr;
        }

        if (encodeConfig) {
            delete encodeConfig;
            encodeConfig = nullptr;
        }
#endif

        if (d3d11Context) {
            d3d11Context->Release();
            d3d11Context = nullptr;
        }

        d3d11Device = nullptr;
        initialized = false;
        lastError = "";
    } catch (...) {
        // 忽略清理时的异常
    }
}

#ifdef NVENC_AVAILABLE
bool NVEncoder::createEncoderSession() {
    try {
        nvencEncoder = new NV_ENCODE_API_FUNCTION_LIST();
        nvencEncoder->version = NV_ENCODE_API_FUNCTION_LIST_VER;

        NVENCSTATUS status = NvEncodeAPICreateInstance(nvencEncoder);
        if (status != NV_ENC_SUCCESS) {
            std::stringstream ss;
            ss << "Failed to create NVENC API instance: " << status;
            lastError = ss.str();
            std::cerr << lastError << std::endl;
            delete nvencEncoder;
            nvencEncoder = nullptr;
            return false;
        }

        NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS openParams = {};
        openParams.version = NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER;
        openParams.device = d3d11Device;
        openParams.deviceType = NV_ENC_DEVICE_TYPE_DIRECTX;
        openParams.apiVersion = NVENCAPI_VERSION;

        void* encoder = nullptr;
        status = nvencEncoder->nvEncOpenEncodeSessionEx(&openParams, &encoder);
        if (status != NV_ENC_SUCCESS) {
            std::stringstream ss;
            ss << "Failed to open NVENC session: " << status;
            lastError = ss.str();
            std::cerr << lastError << std::endl;
            delete nvencEncoder;
            nvencEncoder = nullptr;
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Exception during NVENC session creation: " << e.what();
        lastError = ss.str();
        std::cerr << lastError << std::endl;
        if (nvencEncoder) {
            delete nvencEncoder;
            nvencEncoder = nullptr;
        }
        return false;
    }
}

bool NVEncoder::initializeEncoder() {
    try {
        // 分配初始化参数
        initParams = new NV_ENC_INITIALIZE_PARAMS();
        memset(initParams, 0, sizeof(NV_ENC_INITIALIZE_PARAMS));
        initParams->version = NV_ENC_INITIALIZE_PARAMS_VER;
        initParams->encodeGUID = NV_ENC_CODEC_H264_GUID;
        initParams->presetGUID = NV_ENC_PRESET_LOW_LATENCY_HP_GUID;
        initParams->encodeWidth = width;
        initParams->encodeHeight = height;
        initParams->darWidth = width;
        initParams->darHeight = height;
        initParams->frameRateNum = fps;
        initParams->frameRateDen = 1;
        initParams->enablePTD = 1;
        initParams->reportSliceOffsets = 0;
        initParams->enableSubFrameWrite = 0;

        // 分配编码配置
        encodeConfig = new NV_ENC_CONFIG();
        memset(encodeConfig, 0, sizeof(NV_ENC_CONFIG));
        encodeConfig->version = NV_ENC_CONFIG_VER;
        initParams->encodeConfig = encodeConfig;

        // 设置编码配置
        encodeConfig->profileGUID = NV_ENC_H264_PROFILE_HIGH_GUID;
        encodeConfig->level = NV_ENC_LEVEL_AUTOSELECT;
        encodeConfig->gopLength = fps; // GOP = FPS
        encodeConfig->frameIntervalP = 1; // 无B帧
        encodeConfig->monoChromeEncoding = 0;

        // 设置码率控制
        encodeConfig->rcParams.rateControlMode = NV_ENC_PARAMS_RC_CBR;
        encodeConfig->rcParams.averageBitRate = bitrate * 1000;
        encodeConfig->rcParams.maxBitRate = bitrate * 1000;
        encodeConfig->rcParams.vbvBufferSize = bitrate * 1000 / fps;
        encodeConfig->rcParams.vbvInitialDelay = encodeConfig->rcParams.vbvBufferSize;

        // 设置H.264特定参数
        encodeConfig->encodeCodecConfig.h264Config.idrPeriod = fps;
        encodeConfig->encodeCodecConfig.h264Config.repeatSPSPPS = 1;
        encodeConfig->encodeCodecConfig.h264Config.enableVFR = 0;
        encodeConfig->encodeCodecConfig.h264Config.disableDeblockingFilterIDC = 1;
        encodeConfig->encodeCodecConfig.h264Config.entropyCodingMode = NV_ENC_H264_ENTROPY_CODING_MODE_CABAC;

        NVENCSTATUS status = nvencEncoder->nvEncInitializeEncoder(nvencEncoder, initParams);
        if (status != NV_ENC_SUCCESS) {
            std::stringstream ss;
            ss << "Failed to initialize NVENC encoder: " << status;
            lastError = ss.str();
            std::cerr << lastError << std::endl;
            delete encodeConfig;
            encodeConfig = nullptr;
            delete initParams;
            initParams = nullptr;
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Exception during NVENC encoder initialization: " << e.what();
        lastError = ss.str();
        std::cerr << lastError << std::endl;
        if (encodeConfig) {
            delete encodeConfig;
            encodeConfig = nullptr;
        }
        if (initParams) {
            delete initParams;
            initParams = nullptr;
        }
        return false;
    }
}

bool NVEncoder::createInputResource() {
    try {
        // 创建一个D3D11纹理作为NVENC输入
        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = width;
        texDesc.Height = height;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;

        ID3D11Texture2D* nvencInputTexture = nullptr;
        HRESULT hr = d3d11Device->CreateTexture2D(&texDesc, nullptr, &nvencInputTexture);
        if (FAILED(hr)) {
            std::stringstream ss;
            ss << "Failed to create NVENC input texture: " << hr;
            lastError = ss.str();
            std::cerr << lastError << std::endl;
            return false;
        }

        // 注册纹理到NVENC
        NV_ENC_REGISTER_RESOURCE registerRes = {};
        registerRes.version = NV_ENC_REGISTER_RESOURCE_VER;
        registerRes.resourceType = NV_ENC_INPUT_RESOURCE_TYPE_DIRECTX;
        registerRes.resourceToRegister = nvencInputTexture;
        registerRes.width = width;
        registerRes.height = height;
        registerRes.bufferFormat = NV_ENC_BUFFER_FORMAT_ARGB;

        NVENCSTATUS status = nvencEncoder->nvEncRegisterResource(nvencEncoder, &registerRes, &nvencInputResource);
        if (status != NV_ENC_SUCCESS) {
            std::stringstream ss;
            ss << "Failed to register NVENC input resource: " << status;
            lastError = ss.str();
            std::cerr << lastError << std::endl;
            nvencInputTexture->Release();
            return false;
        }

        // 释放纹理，因为NVENC已经持有了引用
        nvencInputTexture->Release();
        return true;
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Exception during NVENC input resource creation: " << e.what();
        lastError = ss.str();
        std::cerr << lastError << std::endl;
        return false;
    }
}

bool NVEncoder::createBitstreamBuffer() {
    try {
        NV_ENC_CREATE_BITSTREAM_BUFFER createBuffer = {};
        createBuffer.version = NV_ENC_CREATE_BITSTREAM_BUFFER_VER;
        createBuffer.size = width * height * 4; // 足够大的缓冲区

        NVENCSTATUS status = nvencEncoder->nvEncCreateBitstreamBuffer(nvencEncoder, &createBuffer, &nvencBitstreamBuffer);
        if (status != NV_ENC_SUCCESS) {
            std::stringstream ss;
            ss << "Failed to create NVENC bitstream buffer: " << status;
            lastError = ss.str();
            std::cerr << lastError << std::endl;
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Exception during NVENC bitstream buffer creation: " << e.what();
        lastError = ss.str();
        std::cerr << lastError << std::endl;
        return false;
    }
}
#endif

bool NVEncoder::encode(
    void* inputTexture,
    std::vector<uint8_t>& output
) {
    if (!initialized) {
        lastError = "Encoder not initialized";
        return false;
    }

    if (!inputTexture) {
        lastError = "Invalid input texture pointer";
        return false;
    }
    
    // 将void*转换回实际类型
    ID3D11Texture2D* d3dTexture = static_cast<ID3D11Texture2D*>(inputTexture);

#ifdef NVENC_AVAILABLE
    try {
        // 映射输入资源
        NV_ENC_MAP_INPUT_RESOURCE mapRes = {};
        mapRes.version = NV_ENC_MAP_INPUT_RESOURCE_VER;
        mapRes.registeredResource = nvencInputResource;

        NVENCSTATUS status = nvencEncoder->nvEncMapInputResource(nvencEncoder, &mapRes, &nvencMappedResource);
        if (status != NV_ENC_SUCCESS) {
            std::stringstream ss;
            ss << "Failed to map NVENC input resource: " << status;
            lastError = ss.str();
            std::cerr << lastError << std::endl;
            return false;
        }

        // 复制纹理数据
        D3D11_BOX srcBox;
        srcBox.left = 0;
        srcBox.top = 0;
        srcBox.front = 0;
        srcBox.right = width;
        srcBox.bottom = height;
        srcBox.back = 1;

        // 将void*转换回实际类型
        ID3D11DeviceContext* context = static_cast<ID3D11DeviceContext*>(d3d11Context);
        
        context->CopySubresourceRegion(
            static_cast<ID3D11Texture2D*>(nvencMappedResource),
            0,
            0, 0, 0,
            d3dTexture,
            0,
            &srcBox
        );

        // 设置编码参数
        NV_ENC_PIC_PARAMS picParams = {};
        picParams.version = NV_ENC_PIC_PARAMS_VER;
        picParams.inputBuffer = nvencMappedResource;
        picParams.bufferFmt = NV_ENC_BUFFER_FORMAT_ARGB;
        picParams.inputWidth = width;
        picParams.inputHeight = height;
        picParams.pictureStruct = NV_ENC_PIC_STRUCT_FRAME;
        picParams.frameIdx = frameCount++;
        picParams.inputTimeStamp = frameCount;

        // 编码帧
        status = nvencEncoder->nvEncEncodePicture(nvencEncoder, &picParams);
        if (status != NV_ENC_SUCCESS) {
            std::stringstream ss;
            ss << "Failed to encode picture: " << status;
            lastError = ss.str();
            std::cerr << lastError << std::endl;
            
            // 解锁输入资源
            nvencEncoder->nvEncUnmapInputResource(nvencEncoder, nvencMappedResource);
            nvencMappedResource = nullptr;
            return false;
        }

        // 锁定bitstream
        NV_ENC_LOCK_BITSTREAM lockBitstream = {};
        lockBitstream.version = NV_ENC_LOCK_BITSTREAM_VER;
        lockBitstream.outputBitstream = nvencBitstreamBuffer;

        status = nvencEncoder->nvEncLockBitstream(nvencEncoder, &lockBitstream);
        if (status == NV_ENC_SUCCESS) {
            // 复制编码数据到输出缓冲区
            output.resize(lockBitstream.bitstreamSizeInBytes);
            memcpy(output.data(), lockBitstream.bitstreamBufferPtr, lockBitstream.bitstreamSizeInBytes);

            // 解锁bitstream
            nvencEncoder->nvEncUnlockBitstream(nvencEncoder, lockBitstream.outputBitstream);
        } else {
            std::stringstream ss;
            ss << "Failed to lock NVENC bitstream: " << status;
            lastError = ss.str();
            std::cerr << lastError << std::endl;
        }

        // 解锁输入资源
        nvencEncoder->nvEncUnmapInputResource(nvencEncoder, nvencMappedResource);
        nvencMappedResource = nullptr;

        return true;
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Exception during NVENC encoding: " << e.what();
        lastError = ss.str();
        std::cerr << lastError << std::endl;
        
        // 确保资源被释放
        if (nvencMappedResource) {
            try {
                nvencEncoder->nvEncUnmapInputResource(nvencEncoder, nvencMappedResource);
            } catch (...) {
            }
            nvencMappedResource = nullptr;
        }
        
        return false;
    }
#else
    lastError = "NVENC SDK not available";
    return false;
#endif
}
