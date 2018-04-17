#pragma once
#include "Dunya.h"
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

struct ImageRsc
{
  ID3D11Texture2D* pTexture{ nullptr };
  ID3D11ShaderResourceView* pSRV{ nullptr };
};

class UI
{
public:
  UI();
  ~UI();

  bool Initialize(HWND hwnd);
  void Release();

  void Draw();
  void ResizeBuffers(bool release = false);
  void Toggle();
  void Update(double);

  bool IsEnabled() { return m_enabled; }
  bool IsInitialized() { return m_initialized; }
  bool HasKeyboardFocus() { return m_hasKeyboardFocus && m_enabled; }
  bool HasMouseFocus() { return m_hasMouseFocus && m_enabled; }

  HHOOK GetMessageHook() { return hGetMessage; }
  bool HasSeenWarning() { return m_hasSeenWarning; }

  DirectX::XMFLOAT2 GetCursorPosition() { return m_mousePos; }
  HCURSOR GetCursor() { return m_cursor; }

private:
  bool CreateRenderTarget();
  ImageRsc CreateImageFromResource(int resourceID);

private:
  HCURSOR m_cursor;

  bool m_enabled;
  bool m_initialized;

  IDXGISwapChain* m_pSwapChain;
  ID3D11Device* m_pDevice;
  ID3D11DeviceContext* m_pContext;
  ID3D11RenderTargetView* m_pRTV;

  bool m_isResizing;
  int m_framesToSkip;
  double m_resizeTimer;
  double m_swapchainTimeout;

  bool m_hasMouseFocus;
  bool m_hasKeyboardFocus;
  DirectX::XMFLOAT2 m_mousePos;

  HHOOK hGetMessage;
  static LRESULT __stdcall GetMessage_Callback(int nCode, WPARAM wParam, LPARAM lParam);
  static LRESULT __stdcall CallWndProc_Callback(int nCode, WPARAM wParam, LPARAM lParam);

  ImageRsc m_titleImg;
  //ImageWrapper* m_updateBg;
  std::vector<ImageRsc> m_bgImages;
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