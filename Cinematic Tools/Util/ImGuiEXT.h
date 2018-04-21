#pragma once
#include "../imgui/imgui.h"

// Extensions and helpers for fancier ImGui elements
namespace ImGui
{
  IMGUI_API bool ToggleButton(const char* label, const ImVec2& size_arg, bool toggled, bool canUntoggle);
}

