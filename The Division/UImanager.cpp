#include "UIManager.h"
#include "Main.h"
#include "imgui\imgui.h"
#include "imgui\imgui_impl_dx11.h"
#include "Util\ImGuiHelpers.h"
#include "Util\Util.h"
#include "resource.h"

#include <WICTextureLoader.h>
#pragma comment(lib, "DirectXTK.lib")

HHOOK g_getMessageHook = 0;
HCURSOR cursor = NULL;

ID3D11Texture2D* pDepthStencil = nullptr;
ID3D11DepthStencilView* pDepthStencilView = nullptr;

UIManager::UIManager()
{
  util::log::Write("Initializing UI Manager...");
 
  m_pDevice = TD::GameRenderer::GetDevice();
  m_pDevice->GetImmediateContext(&m_pContext);

  CreateBufferRenderTarget();
  
  HWND tdHwnd = FindWindow("Tom Clancy's The DivisionClass", NULL);
  if (!ImGui_ImplDX11_Init((void*)tdHwnd, m_pDevice, m_pContext))
  {
    util::log::Error("Failed to initialize ImGui DX11 handler. GetLastError 0x%X", GetLastError());
    return;
  }

  DWORD thread_id = GetWindowThreadProcessId(tdHwnd, NULL);
  
  if (thread_id)
  {
    if (!(g_getMessageHook = SetWindowsHookEx(WH_GETMESSAGE, this->GetMessage_Callback, g_dllHandle, thread_id)))
      util::log::Error("Couldn't create WH_GETMESSAGE hook. GetLastError 0x%X", GetLastError());
  }
  else
    util::log::Warning("Could not find window thread handle");
  
  void* pData;
  DWORD dataSize;

  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontDefault();

  ImFontConfig config;
  config.OversampleH = 8;
  config.OversampleV = 8;
  config.FontDataOwnedByAtlas = false;

  ImFontConfig configOffset = config;
  configOffset.GlyphOffset = ImVec2(0, -2);

  if (util::GetResource(IDR_FONT_PURISTASEMI, pData, dataSize))
  {
    io.Fonts->AddFontFromMemoryTTF(pData, dataSize, 24, &config);
    io.Fonts->AddFontFromMemoryTTF(pData, dataSize, 20, &configOffset);
    io.Fonts->AddFontFromMemoryTTF(pData, dataSize, 18, &configOffset);
  }
  if (util::GetResource(IDR_FONT_SEGOEUI, pData, dataSize))
    io.Fonts->AddFontFromMemoryTTF(pData, dataSize, 16, &config);

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

  m_titleImg = CreateImageFromResource(IDR_IMG_TITLE);

  ImageRsc bg1 = CreateImageFromResource(IDR_IMG_BG1);
  m_bgImages.push_back(bg1);

  eSelectedMenu = SelectedMenu::UIMenu_Camera;
  m_drawUI = true;
  m_hasKeyboardFocus = false;
  m_hasMouseFocus = false;
  m_isResizing = false;
}

UIManager::~UIManager()
{

}

void UIManager::Draw()
{
  if (m_isResizing)
  {
    m_resizeFrameCounter = m_resizeFrameCounter - 1;

    if (m_resizeFrameCounter <= 0)
    {
      ImGui_ImplDX11_CreateDeviceObjects();
      CreateBufferRenderTarget();
      m_isResizing = false;
    }

    return;
  }

  if (!m_drawUI) return;

  m_pContext->OMSetRenderTargets(1, &m_pRtv, NULL);

  ImGui_ImplDX11_NewFrame();
  {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(800, 460));
    ImGui::Begin("The Division Cinematic Tools", nullptr, ImGuiWindowFlags_NoResize);
    {
      ImVec2 windowPos = ImGui::GetWindowPos();
      ImGui::Dummy(ImVec2(0, 20));

      ImGui::GetWindowDrawList()->AddImage(m_bgImages[0].pShaderResourceView.Get(), ImVec2(windowPos.x, windowPos.y + 19), ImVec2(windowPos.x + 800, windowPos.y + 460));
      //if (m_isFading)
      //  ImGui::GetWindowDrawList()->AddImage(m_bgImages[m_nextIndex].pShaderResourceView.Get(), ImVec2(windowPos.x, windowPos.y + 19), ImVec2(windowPos.x + 800, windowPos.y + 460),
      //    ImVec2(0, 0), ImVec2(1, 1), ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, m_opacity)));

      ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(windowPos.x, windowPos.y + 19), ImVec2(windowPos.x + 800, windowPos.y + 460), 0x66000000);
      ImGui::GetWindowDrawList()->AddImage(m_titleImg.pShaderResourceView.Get(), ImVec2(windowPos.x + 210, windowPos.y + 85), ImVec2(windowPos.x + 592, windowPos.y + 131));

      ImGui::PushFont(io.Fonts->Fonts[2]);
      ImGui::Dummy(ImVec2(232, 33));
      ImGui::SameLine(0, 0);

      if (ImGui::ToggleButton("CAMERA", ImVec2(158, 33), eSelectedMenu == UIMenu_Camera, false))
        eSelectedMenu = UIMenu_Camera;

      ImGui::SameLine(0, 20);
      if (ImGui::ToggleButton("VISUALS", ImVec2(158, 33), eSelectedMenu == UIMenu_Visuals, false))
        eSelectedMenu = UIMenu_Visuals;

      //ImGui::SameLine(0, 20);
      //if (ImGui::ToggleButton("MISC", ImVec2(158, 33), eSelectedMenu == UIMenu_Misc, false))
      //  eSelectedMenu = UIMenu_Misc;

      ImGui::PopFont();

      ImGui::GetWindowDrawList()->AddLine(ImVec2(windowPos.x + 232, windowPos.y + 76), ImVec2(windowPos.x + 390, windowPos.y + 76), 0xFF1C79E5, 2);
      ImGui::GetWindowDrawList()->AddLine(ImVec2(windowPos.x + 410, windowPos.y + 76), ImVec2(windowPos.x + 568, windowPos.y + 76), 0xFF1C79E5, 2);
      //ImGui::GetWindowDrawList()->AddLine(ImVec2(windowPos.x + 499, windowPos.y + 76), ImVec2(windowPos.x + 657, windowPos.y + 76), 0xFF1C79E5, 2);

      ImGui::Dummy(ImVec2(0, 50));
      ImGui::Dummy(ImVec2(0, 0));
      ImGui::SameLine(10);

      {
        ImGui::BeginChild("contentChild", ImVec2(-10, -10), false);

        if (eSelectedMenu == UIMenu_Camera)
          g_mainHandle->GetCameraManager()->DrawUI();
        else if (eSelectedMenu == UIMenu_Visuals)
          g_mainHandle->GetVisualManager()->DrawUI();

        ImGui::EndChild();
      }
      ImGui::End();
    };
  }
  ImGui::Render();

  m_hasKeyboardFocus = ImGui::GetIO().WantCaptureKeyboard;
  m_hasMouseFocus = ImGui::GetIO().WantCaptureMouse;
}

void UIManager::ToggleUI()
{
  m_drawUI = !m_drawUI;
  util::log::Write("UI Enabled: %s", m_drawUI ? "True" : "False");

  TD::ShowMouse(m_drawUI);
}

ImageRsc UIManager::CreateImageFromResource(int resourceId)
{
  ImageRsc newRsc;

  void* pData = nullptr;
  DWORD dwSize = 0;

  if (util::GetResource(resourceId, pData, dwSize))
  {
    HRESULT hr = DirectX::CreateWICTextureFromMemory(m_pDevice, (const uint8_t*)pData, (size_t)dwSize,
      newRsc.pResource.GetAddressOf(),
      newRsc.pShaderResourceView.GetAddressOf());

    if (SUCCEEDED(hr))
      return newRsc;
    else
      util::log::Error("CreateWICTextureFromMemory failed. Result 0x%X GetLastError 0x%X", hr, GetLastError());
  }
  else
    util::log::Error("Could not get resource");

  // In case creating from resource fails, we try to create an empty texture instead

  D3D11_TEXTURE2D_DESC texDesc;
  texDesc.Width = texDesc.Height = 1;
  texDesc.MipLevels = texDesc.ArraySize = 1;
  texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  texDesc.Usage = D3D11_USAGE_DEFAULT;
  texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  texDesc.CPUAccessFlags = texDesc.MiscFlags = 0;

  ID3D11Texture2D* pTexture = nullptr;
  m_pDevice->CreateTexture2D(&texDesc, NULL, &pTexture);
  if (pTexture)
  {
    newRsc.pResource.Attach((ID3D11Resource*)pTexture);
    m_pDevice->CreateShaderResourceView(pTexture, NULL, newRsc.pShaderResourceView.GetAddressOf());
  }

  return newRsc;
}

void UIManager::CreateBufferRenderTarget()

{
  ID3D11Texture2D* pBackBuffer = nullptr;

  HRESULT hr = TD::GameRenderer::Singleton()->m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
  if (FAILED(hr))
  {
    util::log::Error("Failed to retrieve backbuffer from IDXGISwapChain, HRESULT 0x%X", hr);
    return;
  }
  D3D11_TEXTURE2D_DESC backBufferDesc;
  pBackBuffer->GetDesc(&backBufferDesc);

  D3D11_TEXTURE2D_DESC descDepth;
  descDepth.Width = backBufferDesc.Width;
  descDepth.Height = backBufferDesc.Height;
  descDepth.MipLevels = 1;
  descDepth.ArraySize = 1;
  descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  descDepth.SampleDesc.Count = 1;
  descDepth.SampleDesc.Quality = 0;
  descDepth.Usage = D3D11_USAGE_DEFAULT;
  descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  descDepth.CPUAccessFlags = 0;
  descDepth.MiscFlags = 0;
  hr = m_pDevice->CreateTexture2D(&descDepth, NULL, &pDepthStencil);
  if (FAILED(hr))
  {
    util::log::Error("Failed to create DepthStencilBuffer, HRESULT 0x%X", hr);
    return;
  }

  hr = m_pDevice->CreateDepthStencilView(pDepthStencil, NULL, &pDepthStencilView);
  if (FAILED(hr))
  {
    util::log::Error("Failed to create DepthStencilView, HRESULT 0x%X", hr);
    return;
  }

  hr = m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRtv);
  if (FAILED(hr))
  {
    util::log::Error("Failed to create RenderTargetView to backbuffer, HRESULT 0x%X", hr);
    return;
  }

  pBackBuffer->Release();
  pBackBuffer = nullptr;
}

void UIManager::BufferResize()
{
  m_isResizing = true;
  m_resizeFrameCounter = 1000;
  if (m_pRtv)
  {
    util::log::Write("BufferResize");
    ImGui_ImplDX11_InvalidateDeviceObjects();
    m_pRtv->Release();
    m_pRtv = nullptr;
  }
}

void UIManager::Release()
{
  m_drawUI = false;
  ImGui_ImplDX11_Shutdown();
  m_pRtv->Release();
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall UIManager::GetMessage_Callback(int nCode, WPARAM wParam, LPARAM lParam)
{
  if (!g_mainHandle->GetUIManager()->m_isResizing)
  {
    MSG* pMsg = (MSG*)lParam;
    if (pMsg->message == WM_KEYDOWN)
      return CallNextHookEx(g_getMessageHook, nCode, wParam, lParam);

    if (!ImGui_ImplWin32_WndProcHandler(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam))
    {

    }
  }
  
  return CallNextHookEx(g_getMessageHook, nCode, wParam, lParam);
}