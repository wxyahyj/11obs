#include "NVEncoder.h"
#include <iostream>
#include <cstring>

// NVIDIA Video Codec SDK头文件
// 注意：需要安装NVIDIA Video Codec SDK并配置正确的包含路径
#ifdef NVENC_AVAILABLE
    #include <nvEncodeAPI.h>
#endif

NVEncoder::NVEncoder() {
}

NVEncoder::~NVEncoder() {
    cleanup();
}

bool NVEncoder::initialize(
    ID3D11Device* device,
    int w,
    int h,
    int fps_,
    int bitrateKbps
) {
    d3d11Device = device;
    device->GetImmediateContext(&d3d11Context);
    
    width = w;
    height = h;
    fps = fps_;
    bitrate = bitrateKbps;

#ifdef NVENC_AVAILABLE
    if (!createEncoderSession()) {
        std::cerr << "Failed to create NVENC encoder session" << std::endl;
        return false;
    }

    if (!initializeEncoder()) {
        std::cerr << "Failed to initialize NVENC encoder" << std::endl;
        return false;
    }

    if (!createInputResource()) {
        std::cerr << "Failed to create NVENC input resource" << std::endl;
        return false;
    }

    if (!createBitstreamBuffer()) {
        std::cerr << "Failed to create NVENC bitstream buffer" << std::endl;
        return false;
    }

    initialized = true;
    std::cout << "NVEncoder initialized successfully" << std::endl;
    std::cout << "  Resolution: " << width << "x" << height << std::endl;
    std::cout << "  Frame Rate: " << fps << " FPS" << std::endl;
    std::cout << "  Bitrate: " << bitrate << " kbps" << std::endl;
    return true;
#else
    std::cerr << "NVENC SDK not available, encoder will not work" << std::endl;
    return false;
#endif
}

void NVEncoder::cleanup() {
#ifdef NVENC_AVAILABLE
    if (nvencEncoder) {
        auto nvenc = static_cast<NV_ENCODE_API_FUNCTION_LIST*>(nvencEncoder);
        
        if (nvencBitstreamBuffer) {
            nvenc->nvEncDestroyBitstreamBuffer(nvencEncoder, nvencBitstreamBuffer);
            nvencBitstreamBuffer = nullptr;
        }

        if (nvencMappedResource) {
            nvenc->nvEncUnmapInputResource(nvencEncoder, nvencMappedResource);
            nvencMappedResource = nullptr;
        }

        if (nvencInputResource) {
            nvenc->nvEncUnregisterResource(nvencEncoder, nvencInputResource);
            nvencInputResource = nullptr;
        }

        nvenc->nvEncDestroyEncoder(nvencEncoder);
        nvencEncoder = nullptr;
    }

    if (initParams) {
        delete[] static_cast<char*>(initParams);
        initParams = nullptr;
    }

    if (encodeConfig) {
        delete[] static_cast<char*>(encodeConfig);
        encodeConfig = nullptr;
    }
#endif

    if (d3d11Context) {
        d3d11Context->Release();
        d3d11Context = nullptr;
    }

    d3d11Device = nullptr;
    initialized = false;
}

#ifdef NVENC_AVAILABLE
bool NVEncoder::createEncoderSession() {
    NV_ENCODE_API_FUNCTION_LIST* nvenc = new NV_ENCODE_API_FUNCTION_LIST();
    nvenc->version = NV_ENCODE_API_FUNCTION_LIST_VER;

    NVENCSTATUS status = NvEncodeAPICreateInstance(nvenc);
    if (status != NV_ENC_SUCCESS) {
        std::cerr << "Failed to create NVENC API instance: " << status << std::endl;
        delete nvenc;
        return false;
    }

    NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS openParams = {};
    openParams.version = NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER;
    openParams.device = d3d11Device;
    openParams.deviceType = NV_ENC_DEVICE_TYPE_DIRECTX;
    openParams.apiVersion = NVENCAPI_VERSION;

    void* encoder = nullptr;
    status = nvenc->nvEncOpenEncodeSessionEx(&openParams, &encoder);
    if (status != NV_ENC_SUCCESS) {
        std::cerr << "Failed to open NVENC session: " << status << std::endl;
        delete nvenc;
        return false;
    }

    nvencEncoder = nvenc;
    return true;
}

bool NVEncoder::initializeEncoder() {
    auto nvenc = static_cast<NV_ENCODE_API_FUNCTION_LIST*>(nvencEncoder);

    // 分配初始化参数
    NV_ENC_INITIALIZE_PARAMS* init = new NV_ENC_INITIALIZE_PARAMS();
    memset(init, 0, sizeof(NV_ENC_INITIALIZE_PARAMS));
    init->version = NV_ENC_INITIALIZE_PARAMS_VER;
    init->encodeGUID = NV_ENC_CODEC_H264_GUID;
    init->presetGUID = NV_ENC_PRESET_LOW_LATENCY_HP_GUID;
    init->encodeWidth = width;
    init->encodeHeight = height;
    init->darWidth = width;
    init->darHeight = height;
    init->frameRateNum = fps;
    init->frameRateDen = 1;
    init->enablePTD = 1;
    init->reportSliceOffsets = 0;
    init->enableSubFrameWrite = 0;

    // 分配编码配置
    NV_ENC_CONFIG* config = new NV_ENC_CONFIG();
    memset(config, 0, sizeof(NV_ENC_CONFIG));
    config->version = NV_ENC_CONFIG_VER;
    init->encodeConfig = config;

    // 设置编码配置
    config->profileGUID = NV_ENC_H264_PROFILE_HIGH_GUID;
    config->level = NV_ENC_LEVEL_AUTOSELECT;
    config->gopLength = fps; // GOP = FPS
    config->frameIntervalP = 1; // 无B帧
    config->monoChromeEncoding = 0;

    // 设置码率控制
    config->rcParams.rateControlMode = NV_ENC_PARAMS_RC_CBR;
    config->rcParams.averageBitRate = bitrate * 1000;
    config->rcParams.maxBitRate = bitrate * 1000;
    config->rcParams.vbvBufferSize = bitrate * 1000 / fps;
    config->rcParams.vbvInitialDelay = config->rcParams.vbvBufferSize;

    // 设置H.264特定参数
    config->encodeCodecConfig.h264Config.idrPeriod = fps;
    config->encodeCodecConfig.h264Config.repeatSPSPPS = 1;
    config->encodeCodecConfig.h264Config.enableVFR = 0;
    config->encodeCodecConfig.h264Config.disableDeblockingFilterIDC = 1;
    config->encodeCodecConfig.h264Config.entropyCodingMode = NV_ENC_H264_ENTROPY_CODING_MODE_CABAC;

    NVENCSTATUS status = nvenc->nvEncInitializeEncoder(nvencEncoder, init);
    if (status != NV_ENC_SUCCESS) {
        std::cerr << "Failed to initialize NVENC encoder: " << status << std::endl;
        delete config;
        delete init;
        return false;
    }

    initParams = init;
    encodeConfig = config;
    return true;
}

bool NVEncoder::createInputResource() {
    auto nvenc = static_cast<NV_ENCODE_API_FUNCTION_LIST*>(nvencEncoder);

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
        std::cerr << "Failed to create NVENC input texture" << std::endl;
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

    void* resource = nullptr;
    NVENCSTATUS status = nvenc->nvEncRegisterResource(nvencEncoder, &registerRes, &resource);
    if (status != NV_ENC_SUCCESS) {
        std::cerr << "Failed to register NVENC input resource: " << status << std::endl;
        nvencInputTexture->Release();
        return false;
    }

    nvencInputResource = resource;
    return true;
}

bool NVEncoder::createBitstreamBuffer() {
    auto nvenc = static_cast<NV_ENCODE_API_FUNCTION_LIST*>(nvencEncoder);

    NV_ENC_CREATE_BITSTREAM_BUFFER createBuffer = {};
    createBuffer.version = NV_ENC_CREATE_BITSTREAM_BUFFER_VER;
    createBuffer.size = width * height * 4; // 足够大的缓冲区

    void* buffer = nullptr;
    NVENCSTATUS status = nvenc->nvEncCreateBitstreamBuffer(nvencEncoder, &createBuffer, &buffer);
    if (status != NV_ENC_SUCCESS) {
        std::cerr << "Failed to create NVENC bitstream buffer: " << status << std::endl;
        return false;
    }

    nvencBitstreamBuffer = buffer;
    return true;
}
#endif

bool NVEncoder::encode(
    ID3D11Texture2D* inputTexture,
    std::vector<uint8_t>& output
) {
    if (!initialized) {
        return false;
    }

#ifdef NVENC_AVAILABLE
    auto nvenc = static_cast<NV_ENCODE_API_FUNCTION_LIST*>(nvencEncoder);

    // 映射输入资源
    NV_ENC_MAP_INPUT_RESOURCE mapRes = {};
    mapRes.version = NV_ENC_MAP_INPUT_RESOURCE_VER;
    mapRes.registeredResource = nvencInputResource;

    void* mappedRes = nullptr;
    NVENCSTATUS status = nvenc->nvEncMapInputResource(nvencEncoder, &mapRes, &mappedRes);
    if (status != NV_ENC_SUCCESS) {
        std::cerr << "Failed to map NVENC input resource: " << status << std::endl;
        return false;
    }

    nvencMappedResource = mappedRes;

    // 复制纹理数据
    D3D11_TEXTURE2D_DESC texDesc;
    inputTexture->GetDesc(&texDesc);

    D3D11_BOX srcBox;
    srcBox.left = 0;
    srcBox.top = 0;
    srcBox.front = 0;
    srcBox.right = width;
    srcBox.bottom = height;
    srcBox.back = 1;

    d3d11Context->CopySubresourceRegion(
        static_cast<ID3D11Texture2D*>(mappedRes),
        0,
        0, 0, 0,
        inputTexture,
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
    status = nvenc->nvEncEncodePicture(nvencEncoder, &picParams);
    if (status != NV_ENC_SUCCESS) {
        std::cerr << "Failed to encode picture: " << status << std::endl;
        nvenc->nvEncUnmapInputResource(nvencEncoder, nvencMappedResource);
        nvencMappedResource = nullptr;
        return false;
    }

    // 锁定bitstream
    NV_ENC_LOCK_BITSTREAM lockBitstream = {};
    lockBitstream.version = NV_ENC_LOCK_BITSTREAM_VER;
    lockBitstream.outputBitstream = nvencBitstreamBuffer;

    status = nvenc->nvEncLockBitstream(nvencEncoder, &lockBitstream);
    if (status == NV_ENC_SUCCESS) {
        // 复制编码数据到输出缓冲区
        output.resize(lockBitstream.bitstreamSizeInBytes);
        memcpy(output.data(), lockBitstream.bitstreamBufferPtr, lockBitstream.bitstreamSizeInBytes);

        // 解锁bitstream
        nvenc->nvEncUnlockBitstream(nvencEncoder, lockBitstream.outputBitstream);
    }

    // 解锁输入资源
    nvenc->nvEncUnmapInputResource(nvencEncoder, nvencMappedResource);
    nvencMappedResource = nullptr;

    return true;
#else
    // NVENC不可用，返回失败
    return false;
#endif
}
