// dear imgui
// Main implementation file

#include "imgui.h"

// ImGui implementation goes here
// This is a simplified version for demonstration purposes
// In a real project, you would include the full ImGui implementation

// Create context
ImGuiContext* ImGui::CreateContext(ImFontAtlas* shared_font_atlas)
{
    static ImGuiContext ctx;
    return &ctx;
}

// Destroy context
void ImGui::DestroyContext(ImGuiContext* ctx)
{
}

// New frame
void ImGui::NewFrame()
{
}

// Render
void ImGui::Render()
{
}

// Style colors dark
void ImGui::StyleColorsDark()
{
}

// Begin window
bool ImGui::Begin(const char* name, bool* p_open, ImGuiWindowFlags flags)
{
    return true;
}

// End window
void ImGui::End()
{
}

// Text
void ImGui::Text(const char* fmt, ...)
{
}

// Input text
bool ImGui::InputText(const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
    return false;
}

// Input int
bool ImGui::InputInt(const char* label, int* v, int step, int step_fast, ImGuiInputTextFlags flags)
{
    return false;
}

// Button
bool ImGui::Button(const char* label, const ImVec2& size)
{
    return false;
}

// Same line
void ImGui::SameLine(float offset_from_start_x, float spacing)
{
}

// Text colored
void ImGui::TextColored(const ImVec4& col, const char* fmt, ...)
{
}

// Collapsing header
bool ImGui::CollapsingHeader(const char* label, ImGuiTreeNodeFlags flags)
{
    return true;
}

// Separator
void ImGui::Separator()
{
}

// Get IO
ImGuiIO& ImGui::GetIO()
{
    static ImGuiIO io;
    return io;
}

// Get draw data
ImDrawData* ImGui::GetDrawData()
{
    return nullptr;
}

// Check version
void IMGUI_CHECKVERSION()
{
}

// Get mouse cursor
ImGuiMouseCursor ImGui::GetMouseCursor()
{
    return ImGuiMouseCursor_Arrow;
}

// Columns
void ImGui::Columns(int count, const char* id, bool border)
{
}

// Next column
void ImGui::NextColumn()
{
}
