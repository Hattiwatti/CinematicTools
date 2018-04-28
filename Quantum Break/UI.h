#pragma once
#include <d3d11.h>
#include <vector>
#include <wrl.h>

using namespace Microsoft::WRL;

enum SelectedMenu
{
  UIMenu_Camera,
  UIMenu_Visuals,
  UIMenu_Misc
};

struct ImageRsc
{
  ComPtr<ID3D11Texture2D> pTexture{ nullptr };
  ComPtr<ID3D11ShaderResourceView> pSRV{ nullptr };
};

class UI
{
public:
  UI();
  ~UI();

  bool Initialize();

  void Draw();
  void OnResize();
  void Update(double dt);

  bool IsEnabled() { return m_Enabled; }
  bool HasMouseFocus() { return m_Enabled && m_HasMouseFocus; }
  bool HasKeyboardFocus() { return m_Enabled && m_HasKeyboardFocus; }

  HCURSOR GetCursor() { return m_hCursor; }

  void Toggle();
  void ShowUpdateNotes();

private:
  bool CreateRenderTarget();
  ImageRsc CreateImageFromResource(int resourceID);

private:
  bool m_Enabled;
  HCURSOR m_hCursor;
  SelectedMenu m_SelectedMenu;

  ComPtr<ID3D11RenderTargetView> m_pRTV;

  ImageRsc m_TitleImage;
  std::vector<ImageRsc> m_BgImages;
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