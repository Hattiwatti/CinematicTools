#pragma once

#include "Rendering/RenderImpl.h"
#include <DirectXMath.h>
#include <vector>
#include <Windows.h>

enum SelectedMenu
{
  UIMenu_Camera,
  UIMenu_Visuals,
  UIMenu_Visuals_ColorCorrection,
  UIMenu_Visuals_DepthOfField,
  UIMenu_Visuals_Camera,
  UIMenu_Misc
};

class UI
{
public:
  UI();
  ~UI();

  bool Initialize(RendererImpl*);
  void Release();

  void Draw();
  void Toggle();
  void Update(double);

  bool IsEnabled() { return m_enabled; }
  bool IsInitialized() { return m_initialized; }
  bool HasKeyboardFocus() { return m_hasKeyboardFocus && m_enabled; }
  bool HasMouseFocus() { return m_hasMouseFocus && m_enabled; }

  HHOOK GetMessageHook() { return hGetMessage; }
  bool HasSeenWarning() { return m_hasSeenWarning; }

  DirectX::XMFLOAT2 GetCursorPosition() { return m_mousePos; }

private:
  bool m_enabled;
  bool m_initialized;

  RendererImpl* m_pRenderer;

  bool m_hasMouseFocus;
  bool m_hasKeyboardFocus;
  DirectX::XMFLOAT2 m_mousePos;

  HHOOK hGetMessage;
  static LRESULT __stdcall GetMessage_Callback(int nCode, WPARAM wParam, LPARAM lParam);

  ImageWrapper* m_titleImg;
  ImageWrapper* m_updateBg;
  std::vector<ImageWrapper*> m_bgImages;
  int m_bgIndex;
  int m_nextIndex;

  SelectedMenu eSelectedMenu;
  double m_opacity;
  double m_dtFade;
  bool m_isFading;

  DWORD m_lastUpdateWindow;
  bool m_hasSeenWarning;

public:
  UI(UI const&) = delete;
  void operator=(UI const&) = delete;
};

extern HHOOK g_getMessageHook;