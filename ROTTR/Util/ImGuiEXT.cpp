#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGuiEXT.h"
#include "../imgui/imgui_internal.h"

#define IM_F32_TO_INT8_UNBOUND(_VAL)    ((int)((_VAL) * 255.0f + ((_VAL)>=0 ? 0.5f : -0.5f)))   // Unsaturated, for display purpose 
bool ImGui::Checkbox(const char* label, bool* v, const ImVec2& subtractSize)
{
  ImGuiWindow* window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ImGuiContext& g = *GImGui;
  const ImGuiStyle& style = g.Style;
  const ImGuiID id = window->GetID(label);
  const ImVec2 label_size = CalcTextSize(label, NULL, true);

  const ImRect check_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(label_size.y + style.FramePadding.y * 2, label_size.y + style.FramePadding.y * 2)); // We want a square shape to we use Y twice
  ItemSize(check_bb, style.FramePadding.y);

  ImRect total_bb = check_bb;
  if (label_size.x > 0)
    SameLine(0, style.ItemInnerSpacing.x);
  const ImRect text_bb(window->DC.CursorPos + ImVec2(0, style.FramePadding.y), window->DC.CursorPos + ImVec2(0, style.FramePadding.y) + label_size);
  if (label_size.x > 0)
  {
    ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
    total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
  }

  if (!ItemAdd(total_bb, id))
    return false;

  bool hovered, held;
  bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
  if (pressed)
    *v = !(*v);

  RenderFrame(check_bb.Min, check_bb.Max, GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), true, style.FrameRounding);
  if (*v)
  {
    const float check_sz = ImMin(check_bb.GetWidth(), check_bb.GetHeight());
    const float pad = ImMax(1.0f, (float)(int)(check_sz / 6.0f));
    RenderCheckMark(check_bb.Min + ImVec2(pad, pad), GetColorU32(ImGuiCol_CheckMark), check_bb.GetWidth() - pad * 2.0f);
  }

  if (label_size.x > 0.0f)
    RenderText(text_bb.Min, label);

  return pressed;
}

bool ImGui::ToggleButton(const char* label, const ImVec2& size_arg, bool toggled, bool canUntoggle)
{
  ImGuiButtonFlags flags = (toggled && !canUntoggle) ? ImGuiButtonFlags_Disabled : 0;
  int pushedStyles = 0;
  if (toggled)
  {
    PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 1)); pushedStyles++;
    PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.9f, 0.9f, 1)); pushedStyles++;
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
    PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.9f, 0.9f, 1)); pushedStyles++;
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

void ImGui::AlignedText(const char* s)
{
  float availableWidth = ImGui::GetContentRegionAvailWidth();
  ImVec2 textSize = ImGui::CalcTextSize(s);

  ImGui::Dummy(ImVec2(availableWidth / 2 - textSize.x / 2, 0));
  ImGui::SameLine(0, 0);
  ImGui::Text(s);
}

void ImGui::DrawWithBorders(std::function<void()> drawCall)
{
  ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 1, 1, 1));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
  drawCall();
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();
}