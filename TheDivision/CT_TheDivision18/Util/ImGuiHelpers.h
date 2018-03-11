#pragma once
#include "../imgui/imgui.h"

namespace ImGui
{
  IMGUI_API bool ToggleButton(const char* label, const ImVec2& size_arg, bool toggled, bool canUntoggle);
  IMGUI_API void Separator(const ImVec2& size, const ImVec4& color = GetStyleColorVec4(ImGuiCol_Separator));
}