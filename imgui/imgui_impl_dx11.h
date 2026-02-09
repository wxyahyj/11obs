// dear imgui: Renderer Backend for DirectX11
// This needs to be used along with a Platform Backend (e.g. Win32, GLFW, SDL, etc.)

#pragma once
#include "imgui.h"      // IMGUI_IMPL_API

// Data
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;
struct ID3D11ShaderResourceView;

// Backend API
IMGUI_IMPL_API bool     ImGui_ImplDX11_Init(ID3D11Device* device, ID3D11DeviceContext* device_context);
IMGUI_IMPL_API void     ImGui_ImplDX11_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplDX11_NewFrame();
IMGUI_IMPL_API void     ImGui_ImplDX11_RenderDrawData(ImDrawData* draw_data);

// Called by BeginFrame()/NewFrame()
IMGUI_IMPL_API bool     ImGui_ImplDX11_CreateFontsTexture();
IMGUI_IMPL_API void     ImGui_ImplDX11_DestroyFontsTexture();
IMGUI_IMPL_API bool     ImGui_ImplDX11_CreateDeviceObjects();
IMGUI_IMPL_API void     ImGui_ImplDX11_DestroyDeviceObjects();
