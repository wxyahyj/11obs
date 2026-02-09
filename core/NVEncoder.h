#pragma once

#include <d3d11.h>
#include <vector>
#include <nvEncodeAPI.h>

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

    void* nvencEncoder = nullptr;
    NV_ENCODE_API_FUNCTION_LIST nvenc = {};
    NV_ENC_INITIALIZE_PARAMS initParams = {};
    NV_ENC_CONFIG encodeConfig = {};
};
