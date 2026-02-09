// dear imgui: Platform Backend for Windows (standard windows API for 32 and 64-bit applications)
// This needs to be used along with a Renderer (e.g. DirectX11, OpenGL3, Vulkan, WebGPU).

#include "imgui_impl_win32.h"
#include <windows.h>

// Data
static HWND                g_hWnd = NULL;
static DWORD               g_ModifierKeys = 0;
static bool                g_EnableMouseCursor = true;
static ImGuiMouseCursor    g_LastMouseCursor = ImGuiMouseCursor_COUNT;

// Forward declarations of helper functions
static void ImGui_ImplWin32_UpdateMousePos();
static void ImGui_ImplWin32_UpdateMouseButtons();
static void ImGui_ImplWin32_UpdateMouseWheel();

// Helper function to update mouse position
static void ImGui_ImplWin32_UpdateMousePos()
{
    if (g_hWnd == NULL)
        return;

    ImGuiIO& io = ImGui::GetIO();
    POINT pos;
    if (GetCursorPos(&pos))
    {
        // Map from screen to client coordinates
        if (ScreenToClient(g_hWnd, &pos))
        {
            RECT rect;
            GetClientRect(g_hWnd, &rect);
            io.MousePos = ImVec2((float)pos.x, (float)pos.y);
        }
    }
}

// Helper function to update mouse buttons
static void ImGui_ImplWin32_UpdateMouseButtons()
{
    ImGuiIO& io = ImGui::GetIO();

    // Update mouse buttons
    io.MouseDown[0] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    io.MouseDown[1] = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    io.MouseDown[2] = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
}

// Helper function to update mouse wheel
static void ImGui_ImplWin32_UpdateMouseWheel()
{
    ImGuiIO& io = ImGui::GetIO();

    // Update mouse wheel (we don't have a direct way to get mouse wheel delta in Win32)
    // This will be handled by the WndProc handler
}

// ImGui_ImplWin32_Init
bool ImGui_ImplWin32_Init(void* hwnd)
{
    g_hWnd = (HWND)hwnd;

    // Setup back-end capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformName = "imgui_impl_win32";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

    // Keyboard mapping
    io.KeyMap[ImGuiKey_Tab] = VK_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    io.KeyMap[ImGuiKey_Home] = VK_HOME;
    io.KeyMap[ImGuiKey_End] = VK_END;
    io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    io.KeyMap[ImGuiKey_Space] = VK_SPACE;
    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
    io.KeyMap[ImGuiKey_LeftCtrl] = VK_LCONTROL;
    io.KeyMap[ImGuiKey_LeftShift] = VK_LSHIFT;
    io.KeyMap[ImGuiKey_LeftAlt] = VK_LMENU;
    io.KeyMap[ImGuiKey_LeftSuper] = VK_LWIN;
    io.KeyMap[ImGuiKey_RightCtrl] = VK_RCONTROL;
    io.KeyMap[ImGuiKey_RightShift] = VK_RSHIFT;
    io.KeyMap[ImGuiKey_RightAlt] = VK_RMENU;
    io.KeyMap[ImGuiKey_RightSuper] = VK_RWIN;
    io.KeyMap[ImGuiKey_Menu] = VK_APPS;
    io.KeyMap[ImGuiKey_0] = '0';
    io.KeyMap[ImGuiKey_1] = '1';
    io.KeyMap[ImGuiKey_2] = '2';
    io.KeyMap[ImGuiKey_3] = '3';
    io.KeyMap[ImGuiKey_4] = '4';
    io.KeyMap[ImGuiKey_5] = '5';
    io.KeyMap[ImGuiKey_6] = '6';
    io.KeyMap[ImGuiKey_7] = '7';
    io.KeyMap[ImGuiKey_8] = '8';
    io.KeyMap[ImGuiKey_9] = '9';
    io.KeyMap[ImGuiKey_A] = 'A';
    io.KeyMap[ImGuiKey_B] = 'B';
    io.KeyMap[ImGuiKey_C] = 'C';
    io.KeyMap[ImGuiKey_D] = 'D';
    io.KeyMap[ImGuiKey_E] = 'E';
    io.KeyMap[ImGuiKey_F] = 'F';
    io.KeyMap[ImGuiKey_G] = 'G';
    io.KeyMap[ImGuiKey_H] = 'H';
    io.KeyMap[ImGuiKey_I] = 'I';
    io.KeyMap[ImGuiKey_J] = 'J';
    io.KeyMap[ImGuiKey_K] = 'K';
    io.KeyMap[ImGuiKey_L] = 'L';
    io.KeyMap[ImGuiKey_M] = 'M';
    io.KeyMap[ImGuiKey_N] = 'N';
    io.KeyMap[ImGuiKey_O] = 'O';
    io.KeyMap[ImGuiKey_P] = 'P';
    io.KeyMap[ImGuiKey_Q] = 'Q';
    io.KeyMap[ImGuiKey_R] = 'R';
    io.KeyMap[ImGuiKey_S] = 'S';
    io.KeyMap[ImGuiKey_T] = 'T';
    io.KeyMap[ImGuiKey_U] = 'U';
    io.KeyMap[ImGuiKey_V] = 'V';
    io.KeyMap[ImGuiKey_W] = 'W';
    io.KeyMap[ImGuiKey_X] = 'X';
    io.KeyMap[ImGuiKey_Y] = 'Y';
    io.KeyMap[ImGuiKey_Z] = 'Z';
    io.KeyMap[ImGuiKey_F1] = VK_F1;
    io.KeyMap[ImGuiKey_F2] = VK_F2;
    io.KeyMap[ImGuiKey_F3] = VK_F3;
    io.KeyMap[ImGuiKey_F4] = VK_F4;
    io.KeyMap[ImGuiKey_F5] = VK_F5;
    io.KeyMap[ImGuiKey_F6] = VK_F6;
    io.KeyMap[ImGuiKey_F7] = VK_F7;
    io.KeyMap[ImGuiKey_F8] = VK_F8;
    io.KeyMap[ImGuiKey_F9] = VK_F9;
    io.KeyMap[ImGuiKey_F10] = VK_F10;
    io.KeyMap[ImGuiKey_F11] = VK_F11;
    io.KeyMap[ImGuiKey_F12] = VK_F12;

    return true;
}

// ImGui_ImplWin32_Shutdown
void ImGui_ImplWin32_Shutdown()
{
    g_hWnd = NULL;
}

// ImGui_ImplWin32_NewFrame
void ImGui_ImplWin32_NewFrame()
{
    if (g_hWnd == NULL)
        return;

    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate window resizing)
    RECT rect;
    GetClientRect(g_hWnd, &rect);
    io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

    // Setup time
    static DWORD last_time = 0;
    DWORD current_time = GetTickCount();
    io.DeltaTime = (last_time > 0) ? (float)(current_time - last_time) / 1000.0f : 1.0f / 60.0f;
    last_time = current_time;

    // Update mouse position and buttons
    ImGui_ImplWin32_UpdateMousePos();
    ImGui_ImplWin32_UpdateMouseButtons();

    // Update keyboard modifiers
    g_ModifierKeys = 0;
    if (GetKeyState(VK_CONTROL) < 0)
        g_ModifierKeys |= ImGuiMod_Ctrl;
    if (GetKeyState(VK_SHIFT) < 0)
        g_ModifierKeys |= ImGuiMod_Shift;
    if (GetKeyState(VK_MENU) < 0)
        g_ModifierKeys |= ImGuiMod_Alt;
    if (GetKeyState(VK_LWIN) < 0 || GetKeyState(VK_RWIN) < 0)
        g_ModifierKeys |= ImGuiMod_Super;
    io.KeyMods = g_ModifierKeys;

    // Update mouse cursor
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return;

    ImGuiMouseCursor mouse_cursor = ImGui::GetMouseCursor();
    if (mouse_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
    {
        // Hide OS mouse cursor if ImGui is drawing it or if it wants no cursor
        SetCursor(NULL);
        g_LastMouseCursor = ImGuiMouseCursor_COUNT;
    }
    else
    {
        // Show OS mouse cursor
        if (mouse_cursor != g_LastMouseCursor)
        {
            LPCTSTR cursor_name = IDC_ARROW;
            switch (mouse_cursor)
            {
            case ImGuiMouseCursor_Arrow:
                cursor_name = IDC_ARROW;
                break;
            case ImGuiMouseCursor_TextInput:
                cursor_name = IDC_IBEAM;
                break;
            case ImGuiMouseCursor_ResizeAll:
                cursor_name = IDC_SIZEALL;
                break;
            case ImGuiMouseCursor_ResizeNS:
                cursor_name = IDC_SIZENS;
                break;
            case ImGuiMouseCursor_ResizeEW:
                cursor_name = IDC_SIZEWE;
                break;
            case ImGuiMouseCursor_ResizeNESW:
                cursor_name = IDC_SIZENESW;
                break;
            case ImGuiMouseCursor_ResizeNWSE:
                cursor_name = IDC_SIZENWSE;
                break;
            case ImGuiMouseCursor_Hand:
                cursor_name = IDC_HAND;
                break;
            case ImGuiMouseCursor_NotAllowed:
                cursor_name = IDC_NO;
                break;
            }
            SetCursor(LoadCursor(NULL, cursor_name));
            g_LastMouseCursor = mouse_cursor;
        }
    }
}

// ImGui_ImplWin32_WndProcHandler
LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (g_hWnd == NULL)
        g_hWnd = hWnd;

    ImGuiIO& io = ImGui::GetIO();

    switch (msg)
    {
    case WM_LBUTTONDOWN:
        io.MouseDown[0] = true;
        return 0;
    case WM_LBUTTONUP:
        io.MouseDown[0] = false;
        return 0;
    case WM_RBUTTONDOWN:
        io.MouseDown[1] = true;
        return 0;
    case WM_RBUTTONUP:
        io.MouseDown[1] = false;
        return 0;
    case WM_MBUTTONDOWN:
        io.MouseDown[2] = true;
        return 0;
    case WM_MBUTTONUP:
        io.MouseDown[2] = false;
        return 0;
    case WM_MOUSEWHEEL:
        io.MouseWheel += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
        return 0;
    case WM_MOUSEHWHEEL:
        io.MouseWheelH += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
        return 0;
    case WM_MOUSEMOVE:
        io.MousePos = ImVec2((float)LOWORD(lParam), (float)HIWORD(lParam));
        return 0;
    case WM_KEYDOWN:
        if (wParam < 256)
            io.KeysDown[wParam] = true;
        return 0;
    case WM_KEYUP:
        if (wParam < 256)
            io.KeysDown[wParam] = false;
        return 0;
    case WM_CHAR:
        if (wParam > 0 && wParam < 0x10000)
            io.AddInputCharacter((unsigned short)wParam);
        return 0;
    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT && io.MouseDrawCursor)
        {
            SetCursor(NULL);
            return 1;
        }
        return 0;
    default:
        return 0;
    }
}
