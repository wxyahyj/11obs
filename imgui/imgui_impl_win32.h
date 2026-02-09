// dear imgui: Platform Backend for Windows (standard windows API for 32 and 64-bit applications)
// This needs to be used along with a Renderer (e.g. DirectX11, OpenGL3, Vulkan, WebGPU).

#pragma once
#include "imgui.h"      // IMGUI_IMPL_API

// Backend API
IMGUI_IMPL_API bool     ImGui_ImplWin32_Init(void* hwnd);
IMGUI_IMPL_API void     ImGui_ImplWin32_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplWin32_NewFrame();
IMGUI_IMPL_API bool     ImGui_ImplWin32_ProcessMessage(const void* msg);

// Win32 message handler
// You can make this function a member of your class if you want.
IMGUI_IMPL_API LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
