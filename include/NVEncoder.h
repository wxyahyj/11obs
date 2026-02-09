#pragma once

#include <d3d11.h>
#include <vector>
#include <atomic>

// 前向声明NVENC相关类型
typedef int NVENCSTATUS;
typedef void* NvEncoder;
typedef struct {
    unsigned int version;
    // 其他成员将通过运行时动态获取
} NV_ENCODE_API_FUNCTION_LIST;

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
                   unsigned int width, unsigned int height, unsigned int frameRate, unsigned int bitrate);
    bool encodeFrame(ID3D11Texture2D* texture, EncodedFrame& encodedFrame);
    void stop();
    
    unsigned int getWidth() const;
    unsigned int getHeight() const;
    unsigned int getFrameRate() const;
    unsigned int getBitrate() const;
};