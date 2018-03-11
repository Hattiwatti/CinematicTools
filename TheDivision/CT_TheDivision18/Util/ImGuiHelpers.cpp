#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGuiHelpers.h"
#include "../imgui/imgui_internal.h"

bool ImGui::ToggleButton(const char* label, const ImVec2& size_arg, bool toggled, bool canUntoggle)
{
  ImGuiButtonFlags flags = (toggled && !canUntoggle) ? ImGuiButtonFlags_Disabled : 0;
  int pushedStyles = 0;
  if (toggled)
  {
    PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 1)); pushedStyles++;
    PushStyleColor(ImGuiCol_Button, ImVec4(0.9, 0.9, 0.9, 1)); pushedStyles++;
    PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1)); pushedStyles++;
  }

  ImGuiWindow* window = GetCurrentWindow();
  if (window->SkipItems)
  {
    PopStyleColor(pushedStyles);
    return false;
  }

  ImGuiContext& g = *GImGui;
  const ImGuiStyle& style = g.Style;
  const ImGuiID id = window->GetID(label);
  const ImVec2 label_size = CalcTextSize(label, NULL, true);

  ImVec2 pos = window->DC.CursorPos;
  if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrentLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
    pos.y += window->DC.CurrentLineTextBaseOffset - style.FramePadding.y;
  ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

  const ImRect bb(pos, pos + size);
  ItemSize(bb, style.FramePadding.y);
  if (!ItemAdd(bb, id))
  {
    PopStyleColor(pushedStyles);
    return false;
  }

  if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat) flags |= ImGuiButtonFlags_Repeat;
  bool hovered, held;
  bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

  if (pressed && !toggled)
  {
    PushStyleColor(ImGuiCol_Button, ImVec4(0.9, 0.9, 0.9, 1)); pushedStyles++;
    PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1)); pushedStyles++;
  }

  if (held)
  {
    PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1)); pushedStyles++;
  }

  // Render
  ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
  if (pressed) col = ImU32(0xFFFFFFFF);
  RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
  RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

  // Automatically close popups
  //if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
  //    CloseCurrentPopup();

  PopStyleColor(pushedStyles);

  return pressed;
}

void ImGui::Separator(const ImVec2& size, const ImVec4& color)
{
  ImGuiWindow* window = GetCurrentWindow();
  if (window->SkipItems)
    return;
  ImGuiContext& g = *GImGui;

  ImGuiWindowFlags flags = 0;
  if ((flags & (ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_Vertical)) == 0)
    flags |= (window->DC.LayoutType == ImGuiLayoutType_Horizontal) ? ImGuiSeparatorFlags_Vertical : ImGuiSeparatorFlags_Horizontal;
  IM_ASSERT(ImIsPowerOfTwo((int)(flags & (ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_Vertical))));   // Check that only 1 option is selected
  if (flags & ImGuiSeparatorFlags_Vertical)
  {
    VerticalSeparator();
    return;
  }

  // Horizontal Separator
  if (window->DC.ColumnsSet)
    PopClipRect();

  float x1 = window->Pos.x;
  float x2 = window->Pos.x + window->Size.x;
  if (!window->DC.GroupStack.empty())
    x1 += window->DC.IndentX;

  const ImRect bb(ImVec2(window->DC.CursorPos.x, window->DC.CursorPos.y), ImVec2(window->DC.CursorPos.x + size.x, window->DC.CursorPos.y + size.y));
  ItemSize(ImVec2(0.0f, 0.0f)); // NB: we don't provide our width so that it doesn't get feed back into AutoFit, we don't provide height to not alter layout.
  if (!ItemAdd(bb, 0))
  {
    if (window->DC.ColumnsSet)
      PushColumnClipRect();
    return;
  }

  window->DrawList->AddLine(bb.Min, ImVec2(bb.Max.x, bb.Min.y), GetColorU32(ImGuiCol_Separator));

  if (window->DC.ColumnsSet)
  {
    PushColumnClipRect();
    window->DC.ColumnsSet->CellMinY = window->DC.CursorPos.y;
  }
}