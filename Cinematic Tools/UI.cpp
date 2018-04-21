#include "UI.h"
#include "Main.h"
#include "Util/Util.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"

#include <WICTextureLoader.h>
#pragma comment(lib, "DirectXTK.lib")

UI::UI() :
  m_Enabled(false),
  m_FramesToSkip(0),
  m_HasKeyboardFocus(false),
  m_HasMouseFocus(false),
  m_IsResizing(false),
  m_pContext(nullptr),
  m_pDevice(nullptr),
  m_pRTV(nullptr)
{

}

UI::~UI()
{
  ImGui_ImplDX11_Shutdown();
  ImGui::DestroyContext();
}

bool UI::Initialize()
{
  m_pDevice = nullptr; // Get ID3D11Device
  m_pContext = nullptr; // Get ID3D11DeviceContext, or use m_pDevice->GetImmediateContext(&m_pDevice);

  if (!m_pDevice || !m_pContext)
  {
    util::log::Error("Failed to retrieve D3D11 interfaces");
    util::log::Error("m_pDevice 0x%I64X, m_pContext 0x%I64X", m_pDevice, m_pContext);
    return false;
  }

  /////////////////////////
  // ImGui Configuration //
  /////////////////////////

  ImGui::CreateContext();
  if (!ImGui_ImplDX11_Init(g_gameHwnd, m_pDevice, m_pContext))
  {
    util::log::Error("ImGui Dx11 implementation failed to initialize");
    return false;
  }

  ImGuiStyle& Style = ImGui::GetStyle();
  Style.WindowRounding = 0.0f;
  Style.ChildRounding = 0.0f;
  Style.WindowPadding = ImVec2(0, 0);
  Style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
  Style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
  Style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
  Style.Colors[ImGuiCol_Button] = ImVec4(.2f, .2f, .2f, 1.0f);
  Style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.26f, 0.26f, 1.0f);
  Style.Colors[ImGuiCol_ButtonActive] = ImVec4(1, 1, 1, 1);
  Style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0, 0, 0, 0.7f);
  Style.Colors[ImGuiCol_Border] = ImVec4(0, 0, 0, 0);
  Style.Colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

  ///////////////////////////
  // Font loading & config //
  ///////////////////////////



  //////////////////////////////////////
  // Background & title image loading //
  //////////////////////////////////////



  if (!CreateRenderTarget())
    return false;

  util::log::Ok("UI Initialized");
  return true;
}

void UI::Draw()
{
  if (!m_Enabled) return;
  if (m_IsResizing)
  {
    m_FramesToSkip -= 1;
    if (m_FramesToSkip == 0)
    {
      m_IsResizing = false;
      CreateRenderTarget();
    }
    return;
  }

  m_pContext->OMSetRenderTargets(1, &m_pRTV, nullptr);

}

void UI::OnResize()
{
  // Backbuffer needs to be resized when the window size/resolution changes.
  // However it's bound to the UI's RenderTarget (CreateRenderTarget()), so
  // it needs to be released before the backbuffer can be resized.
  // Failing to handle this will usually result in a crash.

  m_IsResizing = true;
  m_FramesToSkip = 1000;
  if (m_pRTV)
    m_pRTV.Reset();
}

void UI::Update(double dt)
{

}

bool UI::CreateRenderTarget()
{
  // Creates a render target to backbuffer resource
  // Should guarantee that stuff actually gets drawn

  IDXGISwapChain* pSwapChain = nullptr; // Get Swapchain here
  ComPtr<ID3D11Texture2D> pBackBuffer = nullptr;

  HRESULT hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)pBackBuffer.ReleaseAndGetAddressOf());
  if (FAILED(hr) || !pBackBuffer)
  {
    util::log::Error("Failed to retrieve backbuffer from SwapChain, HRESULT 0x%X", hr);
    return false;
  }

  hr = m_pDevice->CreateRenderTargetView(pBackBuffer.Get(), NULL, &m_pRTV);
  if (FAILED(hr))
  {
    util::log::Error("CreateRenderTargetView failed, HRESULT 0x%X", hr);
    return false;
  }

  return true;
}

ImageRsc UI::CreateImageFromResource(int resourceID)
{
  ImageRsc newImg;

  DWORD szData;
  void* pData;
  if (!util::GetResource(resourceID, pData, szData))
  {
    util::log::Error("CreateImageFromResource failed");
    return newImg;
  }

  HRESULT hr = DirectX::CreateWICTextureFromMemory(m_pDevice, (const uint8_t*)pData, szData, 
                (ID3D11Resource**)newImg.pTexture.ReleaseAndGetAddressOf(), newImg.pSRV.ReleaseAndGetAddressOf());
  if (FAILED(hr))
    util::log::Error("CreateWICTextureFromMemory failed");

  return newImg;
}