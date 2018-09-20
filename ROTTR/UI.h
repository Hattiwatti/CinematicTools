#pragma once
#include "Rendering/CTRenderer.h"
#include <vector>

using namespace Microsoft::WRL;

enum class SelectedMenu
{
  Camera,
  Visuals_Camera,
  Visuals_DOF,
  Scene,
  Misc
};

class UI
{
public:
  UI();
  ~UI();

  bool Initialize();

  void Draw();
  void Update(double dt);

  bool IsEnabled() { return m_Enabled; }
  bool HasMouseFocus() { return m_Enabled && m_HasMouseFocus; }
  bool HasKeyboardFocus() { return m_Enabled && m_HasKeyboardFocus; }

  void Toggle();
  void ShowUpdateNotes();

private:
  bool m_Enabled;
  SelectedMenu m_SelectedMenu;

  ImgRsc m_TitleImage;
  std::vector<ImgRsc> m_BgImages;
  bool m_IsFadingBg;

  bool m_IsResizing;
  int m_FramesToSkip;

  bool m_HasMouseFocus;
  bool m_HasKeyboardFocus;
  bool m_HasSeenWarning;
  bool m_ShowUpdateNotes;

public:
  UI(UI const&) = delete;
  void operator=(UI const&) = delete;
};