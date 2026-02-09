#pragma once

#include <d3d11.h>

class ScreenCapture {
public:
    ScreenCapture(int width, int height);
    ~ScreenCapture();

    bool initialize();
    ID3D11Texture2D* captureFrame();
    void stop();

    ID3D11Device* device() const { return d3dDevice; }
    ID3D11DeviceContext* deviceContext() const { return d3dContext; }

private:
    ID3D11Device* d3dDevice;
    ID3D11DeviceContext* d3dContext;
    IDXGIOutputDuplication* duplication;
    ID3D11Texture2D* outputTexture;

    int width;
    int height;
    bool running;
};
