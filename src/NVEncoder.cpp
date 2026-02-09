#include "NVEncoder.h"
#include <iostream>
#include <chrono>

// NVENC API函数指针类型
typedef NVENCSTATUS (NVENCAPI* NvEncodeAPICreateInstance)(NV_ENCODE_API_FUNCTION_LIST*);

// 扩展NVEncoder类，添加私有成员
class NVEncoderPrivate {
public:
    ID3D11Device* device;
    ID3D11DeviceContext* deviceContext;
    
    // NVENC相关变量
    void* nvEncoder;
    NV_ENCODE_API_FUNCTION_LIST nvEncodeAPI;
    HMODULE nvEncodeDLL;
    
    NV_ENC_INITIALIZE_PARAMS initializeParams;
    NV_ENC_CONFIG encodeConfig;
    
    // 编码参数
    UINT width;
    UINT height;
    UINT frameRate;
    UINT bitrate;
    
    std::atomic<bool> running;
    
    NVEncoderPrivate()
        : device(nullptr),
          deviceContext(nullptr),
          nvEncoder(nullptr),
          nvEncodeDLL(nullptr),
          width(0),
          height(0),
          frameRate(0),
          bitrate(0),
          running(false) {
        // 初始化NVENC API函数列表
        memset(&nvEncodeAPI, 0, sizeof(NV_ENCODE_API_FUNCTION_LIST));
        nvEncodeAPI.version = NV_ENCODE_API_FUNCTION_LIST_VER;
    }
    
    ~NVEncoderPrivate() {
        stop();
    }
    
    void stop() {
        running = false;
        
        if (nvEncoder && nvEncodeAPI.nvEncCloseEncoder) {
            // 销毁编码器
            nvEncodeAPI.nvEncCloseEncoder(nvEncoder);
            nvEncoder = nullptr;
        }
        
        if (nvEncodeDLL) {
            FreeLibrary(nvEncodeDLL);
            nvEncodeDLL = nullptr;
        }
    }
};

// 实现NVEncoder类
NVEncoder::NVEncoder() {
    d = new NVEncoderPrivate();
}

NVEncoder::~NVEncoder() {
    delete d;
}

bool NVEncoder::initialize(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, 
                          UINT width, UINT height, UINT frameRate, UINT bitrate) {
    d->device = d3dDevice;
    d->deviceContext = d3dContext;
    d->width = width;
    d->height = height;
    d->frameRate = frameRate;
    d->bitrate = bitrate;
    
    // 加载NVENC DLL
    d->nvEncodeDLL = LoadLibrary(L"nvencodeapi64.dll");
    if (!d->nvEncodeDLL) {
        std::cerr << "Failed to load nvencodeapi64.dll" << std::endl;
        return false;
    }
    
    // 获取NVENC API创建函数
    NvEncodeAPICreateInstance createInstance = (NvEncodeAPICreateInstance)GetProcAddress(d->nvEncodeDLL, "NvEncodeAPICreateInstance");
    if (!createInstance) {
        std::cerr << "Failed to get NvEncodeAPICreateInstance" << std::endl;
        d->stop();
        return false;
    }
    
    // 初始化NVENC API函数列表
    NVENCSTATUS status = createInstance(&d->nvEncodeAPI);
    if (status != NV_ENC_SUCCESS) {
        std::cerr << "Failed to create NVENC instance: " << status << std::endl;
        d->stop();
        return false;
    }
    
    // 创建编码器实例
    NV_ENC_OPEN_ENCODER_PARAMS openParams = {
        NV_ENC_OPEN_ENCODER_PARAMS_VER,
        0,
        NV_ENC_DEVICE_TYPE_DIRECTX,
        {0}
    };
    
    // 设置DirectX设备
    openParams.device.d3d11Device = d->device;
    
    status = d->nvEncodeAPI.nvEncOpenEncoder(&d->nvEncoder, &openParams);
    if (status != NV_ENC_SUCCESS) {
        std::cerr << "Failed to open encoder: " << status << std::endl;
        d->stop();
        return false;
    }
    
    // 初始化编码器参数
    memset(&d->initializeParams, 0, sizeof(NV_ENC_INITIALIZE_PARAMS));
    d->initializeParams.version = NV_ENC_INITIALIZE_PARAMS_VER;
    d->initializeParams.encodeGUID = NV_ENC_CODEC_H264_GUID;
    d->initializeParams.presetGUID = NV_ENC_PRESET_LOW_LATENCY_HQ_GUID;
    d->initializeParams.encodeWidth = d->width;
    d->initializeParams.encodeHeight = d->height;
    d->initializeParams.darWidth = d->width;
    d->initializeParams.darHeight = d->height;
    d->initializeParams.frameRateNum = d->frameRate;
    d->initializeParams.frameRateDen = 1;
    d->initializeParams.gopLength = 60; // GOP大小
    d->initializeParams.enablePTD = 1;
    d->initializeParams.enableEncodeAsync = 0; // 禁用异步编码以减少延迟
    
    // 配置编码参数
    memset(&d->encodeConfig, 0, sizeof(NV_ENC_CONFIG));
    d->encodeConfig.version = NV_ENC_CONFIG_VER;
    
    status = d->nvEncodeAPI.nvEncGetEncodeConfig(d->nvEncoder, NV_ENC_CODEC_H264_GUID, &d->encodeConfig);
    if (status != NV_ENC_SUCCESS) {
        std::cerr << "Failed to get encode config: " << status << std::endl;
        d->stop();
        return false;
    }
    
    // 设置低延迟参数
    d->encodeConfig.encodeCodecConfig.h264Config.gopLength = 60;
    d->encodeConfig.encodeCodecConfig.h264Config.idrPeriod = 60;
    d->encodeConfig.encodeCodecConfig.h264Config.enableAdaptiveQuantization = 1;
    d->encodeConfig.encodeCodecConfig.h264Config.bRefMode = 0; // 禁用B帧
    d->encodeConfig.encodeCodecConfig.h264Config.outputBFrames = 0; // 输出B帧数量为0
    d->encodeConfig.encodeCodecConfig.h264Config.rcParams.rateControlMode = NV_ENC_PARAMS_RC_CBR; // CBR模式
    d->encodeConfig.encodeCodecConfig.h264Config.rcParams.averageBitRate = d->bitrate * 1000; // 转换为bps
    d->encodeConfig.encodeCodecConfig.h264Config.rcParams.maxBitRate = d->bitrate * 1000; // 最大码率
    d->encodeConfig.encodeCodecConfig.h264Config.rcParams.vbvBufferSize = d->bitrate * 1000 / 10; // VBV缓冲区大小
    d->encodeConfig.encodeCodecConfig.h264Config.rcParams.vbvInitialDelay = d->bitrate * 1000 / 10;
    d->encodeConfig.encodeCodecConfig.h264Config.rcParams.enableVBV = 1;
    d->encodeConfig.encodeCodecConfig.h264Config.rcParams.enableLookahead = 0; // 禁用前瞻以减少延迟
    d->encodeConfig.encodeCodecConfig.h264Config.rcParams.lookaheadDepth = 0;
    d->encodeConfig.encodeCodecConfig.h264Config.rcParams.disableRateControl = 0;
    
    // 设置零延迟模式
    d->encodeConfig.encodeCodecConfig.h264Config.rcParams.enableAQ = 1;
    d->encodeConfig.encodeCodecConfig.h264Config.rcParams.aqStrength = 15;
    
    d->initializeParams.encodeConfig = &d->encodeConfig;
    
    // 初始化编码器
    status = d->nvEncodeAPI.nvEncInitializeEncoder(d->nvEncoder, &d->initializeParams);
    if (status != NV_ENC_SUCCESS) {
        std::cerr << "Failed to initialize encoder: " << status << std::endl;
        d->stop();
        return false;
    }
    
    d->running = true;
    return true;
}

bool NVEncoder::encodeFrame(ID3D11Texture2D* texture, EncodedFrame& encodedFrame) {
    if (!d->running || !d->nvEncoder) {
        return false;
    }
    
    // 注册输入资源
    NV_ENC_REGISTER_RESOURCE registerResource = {
        NV_ENC_REGISTER_RESOURCE_VER,
        NV_ENC_INPUT_RESOURCE_TYPE_DIRECTX,
        {0}
    };
    
    registerResource.resourceToRegister.d3d11Resource = texture;
    
    NVENCSTATUS status = d->nvEncodeAPI.nvEncRegisterResource(d->nvEncoder, &registerResource);
    if (status != NV_ENC_SUCCESS) {
        std::cerr << "Failed to register resource: " << status << std::endl;
        return false;
    }
    
    // 创建输入帧
    NV_ENC_PIC_PARAMS picParams = {
        NV_ENC_PIC_PARAMS_VER
    };
    
    picParams.inputBuffer = registerResource.registeredResource;
    picParams.inputWidth = d->width;
    picParams.inputHeight = d->height;
    picParams.inputPitch = d->width * 4; // RGBA格式
    picParams.pictureStruct = NV_ENC_PICTURE_STRUCT_FRAME;
    picParams.encodePicFlags = NV_ENC_PIC_FLAG_OUTPUT_SPSPPS;
    
    // 编码帧
    status = d->nvEncodeAPI.nvEncEncodePicture(d->nvEncoder, &picParams);
    if (status != NV_ENC_SUCCESS) {
        std::cerr << "Failed to encode picture: " << status << std::endl;
        d->nvEncodeAPI.nvEncUnregisterResource(d->nvEncoder, registerResource.registeredResource);
        return false;
    }
    
    // 获取编码输出
    NV_ENC_LOCK_BITSTREAM lockBitstream = {
        NV_ENC_LOCK_BITSTREAM_VER
    };
    
    status = d->nvEncodeAPI.nvEncLockBitstream(d->nvEncoder, picParams.outputBitstream, &lockBitstream);
    if (status != NV_ENC_SUCCESS) {
        std::cerr << "Failed to lock bitstream: " << status << std::endl;
        d->nvEncodeAPI.nvEncUnregisterResource(d->nvEncoder, registerResource.registeredResource);
        return false;
    }
    
    // 复制编码数据
    encodedFrame.data.resize(lockBitstream.bitstreamSizeInBytes);
    memcpy(encodedFrame.data.data(), lockBitstream.bitstreamBufferPtr, lockBitstream.bitstreamSizeInBytes);
    encodedFrame.frameId = picParams.pictureIndex;
    encodedFrame.timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()
    ).count();
    
    // 解锁位流
    d->nvEncodeAPI.nvEncUnlockBitstream(d->nvEncoder, picParams.outputBitstream);
    
    // 注销资源
    d->nvEncodeAPI.nvEncUnregisterResource(d->nvEncoder, registerResource.registeredResource);
    
    return true;
}

void NVEncoder::stop() {
    d->stop();
}

UINT NVEncoder::getWidth() const { return d->width; }
UINT NVEncoder::getHeight() const { return d->height; }
UINT NVEncoder::getFrameRate() const { return d->frameRate; }
UINT NVEncoder::getBitrate() const { return d->bitrate; }