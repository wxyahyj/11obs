#include "MainWindow.h"
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <string>
#include <sstream>

using namespace std;

MainWindow::MainWindow() {
}

MainWindow::~MainWindow() {
}

void MainWindow::draw(StreamConfig& config, StreamController& controller) {
    ImGui::Begin("UDP Video Sender");

    // 配置面板
    if (ImGui::CollapsingHeader("Configuration", ImGuiTreeNodeFlags_DefaultOpen)) {
        drawConfigPanel(config);
    }

    ImGui::Separator();

    // 控制面板
    drawControlPanel(config, controller);

    ImGui::Separator();

    // 统计面板
    if (ImGui::CollapsingHeader("Statistics", ImGuiTreeNodeFlags_DefaultOpen)) {
        drawStatsPanel(controller);
    }

    ImGui::End();
}

void MainWindow::drawConfigPanel(StreamConfig& config) {
    // 网络配置
    ImGui::Text("Network Configuration");
    ImGui::InputText("Target IP", config.targetIp, sizeof(config.targetIp));
    ImGui::InputInt("Port", &config.port, 1, 100);
    ImGui::Spacing();

    // 视频配置
    ImGui::Text("Video Configuration");
    ImGui::InputInt("Width", &config.width, 32, 128);
    ImGui::InputInt("Height", &config.height, 32, 128);
    ImGui::InputInt("FPS", &config.fps, 10, 50);
    ImGui::InputInt("Bitrate (kbps)", &config.bitrateKbps, 1000, 5000);
    ImGui::Spacing();

    // 性能配置
    ImGui::Text("Performance Configuration");
    ImGui::InputInt("Capture Queue Size", &config.captureQueueSize, 1, 5);
    ImGui::InputInt("Encode Queue Size", &config.encodeQueueSize, 1, 5);

    // 限制范围
    if (config.width < 64) config.width = 64;
    if (config.width > 4096) config.width = 4096;
    if (config.height < 64) config.height = 64;
    if (config.height > 4096) config.height = 4096;
    if (config.fps < 1) config.fps = 1;
    if (config.fps > 240) config.fps = 240;
    if (config.bitrateKbps < 1000) config.bitrateKbps = 1000;
    if (config.bitrateKbps > 50000) config.bitrateKbps = 50000;
    if (config.captureQueueSize < 1) config.captureQueueSize = 1;
    if (config.captureQueueSize > 10) config.captureQueueSize = 10;
    if (config.encodeQueueSize < 1) config.encodeQueueSize = 1;
    if (config.encodeQueueSize > 10) config.encodeQueueSize = 10;
}

void MainWindow::drawControlPanel(StreamConfig& config, StreamController& controller) {
    ImGui::Text("Stream Control");

    bool isRunning = controller.isRunning();

    if (isRunning) {
        if (ImGui::Button("Stop Streaming")) {
            controller.stop();
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Running");
    } else {
        if (ImGui::Button("Start Streaming")) {
            controller.start(config);
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Stopped");
    }
}

void MainWindow::drawStatsPanel(StreamController& controller) {
    ImGui::Text("Performance Statistics");

    // FPS统计
    ImGui::Columns(3, "StatsColumns", false);
    ImGui::Separator();

    ImGui::Text("Capture FPS");
    ImGui::NextColumn();
    ImGui::Text("Encode FPS");
    ImGui::NextColumn();
    ImGui::Text("Send FPS");
    ImGui::NextColumn();

    ImGui::Separator();

    ImGui::Text("%d", controller.getCaptureFPS());
    ImGui::NextColumn();
    ImGui::Text("%d", controller.getEncodeFPS());
    ImGui::NextColumn();
    ImGui::Text("%d", controller.getSendFPS());
    ImGui::NextColumn();

    ImGui::Columns(1);
    ImGui::Spacing();

    // 网络统计
    ImGui::Text("Network Statistics");
    ImGui::Columns(2, "NetworkColumns", false);
    ImGui::Separator();

    ImGui::Text("Bytes Sent");
    ImGui::NextColumn();
    ImGui::Text("Packets Sent");
    ImGui::NextColumn();

    ImGui::Separator();

    // 格式化字节数
    int bytesSent = controller.getBytesSent();
    std::string bytesStr;
    if (bytesSent < 1024) {
        bytesStr = std::to_string(bytesSent) + " B";
    } else if (bytesSent < 1024 * 1024) {
        bytesStr = std::to_string(bytesSent / 1024) + " KB";
    } else if (bytesSent < 1024 * 1024 * 1024) {
        bytesStr = std::to_string(bytesSent / (1024 * 1024)) + " MB";
    } else {
        bytesStr = std::to_string(bytesSent / (1024 * 1024 * 1024)) + " GB";
    }

    ImGui::Text("%s", bytesStr.c_str());
    ImGui::NextColumn();
    ImGui::Text("%d", controller.getPacketsSent());
    ImGui::NextColumn();

    ImGui::Columns(1);
}
