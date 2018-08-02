#pragma once
#include "Rendering/CTRenderer.h"
#include <vector>

using namespace Microsoft::WRL;

enum SelectedMenu
{
  UIMenu_Camera,
  UIMenu_Visuals,
  UIMenu_Misc
};


class UI
{
public:
  UI();
  ~UI();

  bool Initialize();

  void Draw();
  void OnResize();
  void Update(float dt);

  bool IsEnabled() { return m_Enabled; }
  bool HasMouseFocus() { return m_Enabled && m_HasMouseFocus; }
  bool HasKeyboardFocus() { return m_Enabled && m_HasKeyboardFocus; }

  void Toggle();
  void ShowUpdateNotes();

  void BindRenderTarget();

private:
  bool CreateRenderTarget();

private:
  bool m_Enabled;
  SelectedMenu m_SelectedMenu;

  ComPtr<ID3D11RenderTargetView> m_pRTV;

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