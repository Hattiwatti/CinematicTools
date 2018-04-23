#pragma once
#include "Modules\Snowdrop.h"
#include <wrl.h>

enum SelectedMenu
{
  UIMenu_Camera,
  UIMenu_Visuals,
  UIMenu_Visuals_ColorCorrection,
  UIMenu_Visuals_DepthOfField,
  UIMenu_Misc
};

struct ImageRsc
{
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pShaderResourceView;
  Microsoft::WRL::ComPtr<ID3D11Resource> pResource;
};

class UIManager
{
public:
  UIManager();
  ~UIManager();

  void Release();
  void Draw();

  void BufferResize();

  void ToggleUI();

  bool IsUIEnabled() { return m_drawUI; }
  bool HasKeyboardFocus() { return m_hasKeyboardFocus && m_drawUI; }

private:
  ImageRsc CreateImageFromResource(int resourceId);

  void CreateBufferRenderTarget();

private:
  bool m_drawUI;
  bool m_isResizing;
  int m_resizeFrameCounter;

  bool m_hasKeyboardFocus;
  bool m_hasMouseFocus;

  ID3D11RenderTargetView* m_pRtv;
  ID3D11DepthStencilView* m_pDepthStencilView;

  ID3D11Device* m_pDevice;
  ID3D11DeviceContext* m_pContext;

  static LRESULT __stdcall GetMessage_Callback(int nCode, WPARAM wParam, LPARAM lParam);

  ImageRsc m_titleImg;
  std::vector<ImageRsc> m_bgImages;
  int m_bgIndex;
  int m_nextIndex;

  SelectedMenu eSelectedMenu;
  double m_opacity;
  double m_dtFade;
  bool m_isFading;

public:
  UIManager(UIManager const&) = delete;
  void operator=(UIManager const&) = delete;
};