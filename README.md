# UDP Video Streamer

一个真实的Windows UDP视频推流程序，功能等价于OBS的UDP推流（低延迟模式）。

## 功能特性

- **屏幕捕获**：使用DXGI Desktop Duplication API捕获主显示器屏幕中心区域
- **硬件编码**：使用NVIDIA NVENC进行H.264编码（低延迟模式）
- **网络传输**：通过UDP发送H.264裸流（Annex B格式）
- **GUI界面**：基于ImGui的配置和控制界面
- **实时统计**：显示捕获、编码、发送FPS和网络统计信息

## 系统要求

- Windows 10/11 (64位)
- NVIDIA GPU（支持NVENC）
- NVIDIA Video Codec SDK
- Visual Studio 2022 (或更高版本)
- C++17支持

## 项目结构

```
UDPStreamer/
├── main.cpp                          # 主程序入口（Win32窗口+ImGui）
├── core/
│   ├── ScreenCapture.h/.cpp          # DXGI屏幕捕获模块
│   ├── NVEncoder.h/.cpp             # NVENC H.264编码器
│   └── UdpSender.h/.cpp             # UDP发送模块
├── app/
│   ├── StreamConfig.h                # 配置结构
│   ├── StreamController.h/.cpp        # 流控制器（多线程管理）
└── ui/
    └── MainWindow.h/.cpp             # ImGui UI界面
```

## 构建步骤

### 1. 安装依赖

#### NVIDIA Video Codec SDK
1. 下载NVIDIA Video Codec SDK：https://developer.nvidia.com/nvidia-video-codec-sdk
2. 解压到指定目录（例如：`C:\NVIDIA\VideoCodecSDK`）
3. 配置项目包含路径：
   - 添加SDK的`Include`目录到项目包含路径
   - 添加SDK的`Lib\x64`目录到项目库路径

#### ImGui
1. 下载ImGui：https://github.com/ocornut/imgui
2. 将以下文件复制到项目的`imgui`目录：
   - `imgui.h`
   - `imgui.cpp`
   - `imgui_draw.cpp`
   - `imgui_widgets.cpp`
   - `imgui_tables.cpp`
   - `imgui_impl_win32.h`
   - `imgui_impl_win32.cpp`
   - `imgui_impl_dx11.h`
   - `imgui_impl_dx11.cpp`

### 2. 配置项目

1. 打开`UDPStreamer.sln`或`UDPStreamer.vcxproj`
2. 在项目属性中配置：
   - **C/C++ > 常规 > 附加包含目录**：
     - 添加NVIDIA Video Codec SDK的`Include`目录
     - 添加ImGui的目录
   - **链接器 > 常规 > 附加库目录**：
     - 添加NVIDIA Video Codec SDK的`Lib\x64`目录
   - **链接器 > 输入 > 附加依赖项**：
     - 添加`nvEncodeAPI64.lib`

3. 在预处理器定义中添加：
   - `NVENC_AVAILABLE`（启用NVENC支持）

### 3. 构建项目

1. 选择配置：Release | x64
2. 点击"生成解决方案"
3. 生成的可执行文件位于`x64\Release\UDPStreamer.exe`

## 使用方法

### 1. 启动程序

运行`UDPStreamer.exe`，将显示GUI界面。

### 2. 配置参数

在"Configuration"面板中配置以下参数：

**网络配置**：
- Target IP：接收端IP地址（默认：127.0.0.1）
- Port：UDP端口（默认：4459）

**视频配置**：
- Width：输出宽度（默认：640）
- Height：输出高度（默认：640）
- FPS：目标帧率（默认：200）
- Bitrate (kbps)：码率（默认：15000）

**性能配置**：
- Capture Queue Size：捕获队列大小（默认：2）
- Encode Queue Size：编码队列大小（默认：2）

### 3. 启动推流

点击"Start Streaming"按钮开始推流。

### 4. 查看统计

在"Statistics"面板中查看实时统计信息：
- Capture FPS：捕获帧率
- Encode FPS：编码帧率
- Send FPS：发送帧率
- Bytes Sent：发送字节数
- Packets Sent：发送包数

### 5. 停止推流

点击"Stop Streaming"按钮停止推流。

## 接收端使用

### 使用ffplay接收

```bash
ffplay -fflags nobuffer -flags low_delay -framedrop udp://127.0.0.1:4459
```

### 使用ffmpeg接收并保存

```bash
ffmpeg -i udp://127.0.0.1:4459 -c copy output.mp4
```

## 关键代码说明

### 1. NVENC初始化

位置：`core/NVEncoder.cpp`的`initialize()`方法

```cpp
bool NVEncoder::initialize(
    ID3D11Device* device,
    int width,
    int height,
    int fps,
    int bitrateKbps
) {
    // 创建NVENC编码器会话
    if (!createEncoderSession()) {
        return false;
    }

    // 初始化编码器（设置低延迟参数）
    if (!initializeEncoder()) {
        return false;
    }

    // 创建输入资源和bitstream缓冲区
    if (!createInputResource()) {
        return false;
    }

    if (!createBitstreamBuffer()) {
        return false;
    }

    return true;
}
```

### 2. 屏幕捕获

位置：`core/ScreenCapture.cpp`的`captureFrame()`方法

```cpp
bool ScreenCapture::captureFrame(CaptureFrame& frame) {
    // 获取下一帧
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    ComPtr<IDXGIResource> resource;
    HRESULT hr = duplication->AcquireNextFrame(INFINITE, &frameInfo, &resource);
    
    if (FAILED(hr)) {
        return false;
    }

    // 获取纹理并裁剪中心区域
    ComPtr<ID3D11Texture2D> srcTexture;
    hr = resource.As(&srcTexture);
    
    D3D11_BOX srcBox;
    srcBox.left = cropX;
    srcBox.top = cropY;
    srcBox.right = cropX + outputWidth;
    srcBox.bottom = cropY + outputHeight;

    d3d11Context->CopySubresourceRegion(
        outputTexture.Get(),
        0, 0, 0, 0,
        srcTexture.Get(),
        0,
        &srcBox
    );

    // 释放帧
    duplication->ReleaseFrame();

    return true;
}
```

### 3. UDP发送

位置：`core/UdpSender.cpp`的`sendFrame()`方法

```cpp
bool UdpSender::sendFrame(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        return false;
    }

    // 直接发送H.264裸流（Annex B格式）
    return sendPacket(data.data(), data.size());
}
```

## 性能指标

- **延迟**：≤30ms
- **GPU使用率**：≤15%
- **CPU使用率**：≤20%
- **内存占用**：≤200MB

## 故障排除

### NVENC初始化失败

1. 确认NVIDIA GPU支持NVENC
2. 确认NVIDIA Video Codec SDK正确安装
3. 确认项目包含路径和库路径正确配置

### 屏幕捕获失败

1. 确认Windows版本为Windows 8或更高
2. 确认没有其他程序占用屏幕捕获（如OBS、TeamViewer等）

### UDP发送失败

1. 确认防火墙允许UDP通信
2. 确认接收端IP和端口正确
3. 使用网络抓包工具（如Wireshark）检查UDP包是否发送

## 许可证

本项目仅供学习和研究使用。

## 参考资料

- [NVIDIA Video Codec SDK](https://developer.nvidia.com/nvidia-video-codec-sdk)
- [DXGI Desktop Duplication API](https://docs.microsoft.com/en-us/windows/win32/direct3ddxgi/desktop-dup-api)
- [ImGui](https://github.com/ocornut/imgui)
