#include "MainWindow.h"
#include <imgui.h>

void DrawMainWindow(StreamConfig& cfg, StreamController& controller) {
    ImGui::Begin("UDP Video Sender");

    ImGui::InputText("Target IP", cfg.targetIp, sizeof(cfg.targetIp));
    ImGui::InputInt("Port", &cfg.port);

    ImGui::Separator();

    ImGui::InputInt("Width", &cfg.width);
    ImGui::InputInt("Height", &cfg.height);

    ImGui::InputInt("FPS", &cfg.fps);
    ImGui::InputInt("Bitrate (kbps)", &cfg.bitrateKbps);

    ImGui::Separator();

    if (!controller.isRunning()) {
        if (ImGui::Button("Start Streaming")) {
            controller.start(cfg);
        }
    } else {
        ImGui::BeginDisabled();
        ImGui::Button("Start Streaming");
        ImGui::EndDisabled();
    }

    ImGui::SameLine();

    if (controller.isRunning()) {
        if (ImGui::Button("Stop")) {
            controller.stop();
        }
    } else {
        ImGui::BeginDisabled();
        ImGui::Button("Stop");
        ImGui::EndDisabled();
    }

    ImGui::Text("Status: %s",
        controller.isRunning() ? "Streaming" : "Idle");

    ImGui::End();
}
