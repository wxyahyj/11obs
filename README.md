# 极低延迟直播推流软件

这是一款专为Windows 10/11（64位）平台设计的极低延迟直播推流软件，目标是在延迟、稳定性和画质表现上达到OBS低延迟模式水平，但不依赖OBS及其相关组件。

## 功能特点

- **极低延迟**：采用丢帧策略而非卡顿缓冲，确保实时性
- **GPU硬编码**：使用NVIDIA NVENC H.264硬件编码器，降低CPU占用
- **UDP传输**：使用自定义轻量级UDP协议，确保数据实时性
- **多线程架构**：三线程设计（采集、编码、传输），使用无锁队列进行线程间通信
- **资源占用优化**：最小化CPU与GPU资源消耗

## 技术架构

- **屏幕采集**：使用DXGI Desktop Duplication API，支持640×640中心裁剪
- **视频编码**：使用NVIDIA NVENC H.264硬件编码器，配置低延迟参数
- **网络传输**：使用UDP协议，严格限制数据包大小≤1400字节

## 系统要求

- **操作系统**：Windows 10/11（64位）
- **GPU**：NVIDIA RTX 4060（支持NVENC H.264编码）
- **CPU**：Intel 12代i5/i3（H系列移动处理器）
- **内存**：至少8GB RAM
- **网络**：支持UDP协议的局域网或公网环境，建议带宽≥20Mbps

## 编译环境

- **Visual Studio**：2019或更高版本
- **Windows SDK**：10.0.19041.0或更高
- **DirectX SDK**：包含在Windows SDK中
- **NVIDIA驱动**：最新版本（支持NVENC）

## 使用方法

### 命令行参数

```bash
LowLatencyStreamer.exe [options]

Options:
  --display <index>    设置显示器索引（默认：0）
  --width <width>      设置输出宽度（默认：640）
  --height <height>    设置输出高度（默认：640）
  --fps <rate>         设置帧率（默认：200）
  --bitrate <rate>     设置码率（默认：15000）
  --server <ip>        设置服务器IP（默认：127.0.0.1）
  --port <port>        设置服务器端口（默认：5000）
  --max-packet-size <size>  设置最大数据包大小（默认：1400）
```

### 示例

```bash
# 使用默认配置推流到本地服务器
LowLatencyStreamer.exe

# 自定义配置
LowLatencyStreamer.exe --width 1280 --height 720 --fps 60 --bitrate 10000 --server 192.168.1.100 --port 5000
```

## 项目结构

```
LowLatencyStreamer/
├── include/                 # 头文件目录
│   ├── ScreenCapture.h      # 屏幕采集模块
│   ├── NVEncoder.h          # 视频编码模块
│   ├── UDPTransmitter.h     # 网络传输模块
│   ├── LockFreeQueue.h      # 无锁队列
│   ├── LiveStreamer.h       # 主控制模块
│   └── ConfigManager.h      # 配置管理模块
├── src/                     # 源代码目录
│   ├── main.cpp             # 主入口文件
│   ├── ScreenCapture.cpp    # 屏幕采集实现
│   ├── NVEncoder.cpp        # 视频编码实现
│   ├── UDPTransmitter.cpp   # 网络传输实现
│   ├── LiveStreamer.cpp     # 主控制实现
│   └── ConfigManager.cpp    # 配置管理实现
├── .github/                 # GitHub工作流
│   └── workflows/           # 工作流目录
│       └── build.yml        # 构建工作流
├── LowLatencyStreamer.sln   # Visual Studio解决方案
└── README.md                # 项目说明
```

## 性能目标

- **端到端延迟**：≤30ms
- **编码延迟**：≤5ms
- **GPU使用率**：≤15%
- **CPU使用率**：≤20%
- **内存占用**：≤200MB
- **稳定性**：连续运行24小时以上无崩溃

## 注意事项

1. 本软件仅支持直播推流，不包含录制、回放或其他附加功能
2. 在GitHub Actions环境中，由于可能没有安装NVIDIA驱动和NVENC SDK，视频编码功能会使用简化实现
3. 实际使用时，建议使用支持NVENC的NVIDIA显卡以获得最佳性能

## 许可证

本项目采用MIT许可证。
