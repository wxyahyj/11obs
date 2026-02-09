#pragma once

#include <d3d11.h>
#include <vector>

// 尝试包含NVENC头文件，如果不可用则使用简化实现
#ifdef _WIN32
    #ifdef NVENC_AVAILABLE
        #include <nvEncodeAPI.h>
    #endif
#endif

class NVEncoder {
public:
    NVEncoder(
        ID3D11Device* device,
        int width,
        int height,
        int fps,
        int bitrateKbps
    );

    ~NVEncoder();

    bool encode(
        ID3D11Texture2D* inputTexture,
        std::vector<uint8_t>& output
    );

private:
    bool initNVENC();

private:
    ID3D11Device* d3d11Device;
    int width;
    int height;
    int fps;
    int bitrate;

#ifdef NVENC_AVAILABLE
    void* nvencEncoder = nullptr;
    NV_ENCODE_API_FUNCTION_LIST nvenc = {};
    NV_ENC_INITIALIZE_PARAMS initParams = {};
    NV_ENC_CONFIG encodeConfig = {};
#endif
};
