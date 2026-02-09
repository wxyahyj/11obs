#pragma once

#include <d3d11.h>
#include <vector>
#include <atomic>

// NVENC SDK相关头文件
#include <nvEncodeAPI.h>

class NVEncoderPrivate;

class NVEncoder {
private:
    NVEncoderPrivate* d;
    
public:
    struct EncodedFrame {
        std::vector<uint8_t> data;
        uint32_t frameId;
        uint64_t timestamp;
    };
    
    NVEncoder();
    ~NVEncoder();
    
    bool initialize(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, 
                   UINT width, UINT height, UINT frameRate, UINT bitrate);
    bool encodeFrame(ID3D11Texture2D* texture, EncodedFrame& encodedFrame);
    void stop();
    
    UINT getWidth() const;
    UINT getHeight() const;
    UINT getFrameRate() const;
    UINT getBitrate() const;
};