#include "UIManager.h"
#include "Main.h"
#include "resource.h"
#include "ImGui/imgui_impl_dx11.h"
#include "Util/Util.h"
#include "Util/ImGuiHelpers.h"
#include <WICTextureLoader.h>

#pragma comment(lib, "DirectXTK.lib")

HHOOK g_getMessageHook = 0;
HHOOK g_callWndProcHook = 0;
bool g_isResizing = false;
float g_timeScale = 1.f;

const char* g_creditsText = "The following libraries are used in making the Cinematic Tools:\n"
"MinHook by Tsuda Kageyu, inih by Ben Hoyt, ImGui by Omar Cornut, C++ libraries by Boost, DirectXTK and DirectXTex by Microsoft.\n\n"
"Thank you for using the tools and making awesome stuff with it!\n\n"
"Remember to report bugs at www.cinetools.xyz/bugs";

UI::UI()
{
  m_initialized = false;
  m_isResizing = false;
  m_framesToSkip = 0;
  m_swapchainTimeout = 0;
  m_resizeTimer = 0;

  m_enabled = false;
  m_hasKeyboardFocus = false;
  m_hasMouseFocus = false;
  eSelectedMenu = UIMenu_Camera;
  m_dtFade = 0;

  m_pRTV = nullptr;
  m_pDevice = nullptr;
  m_pContext = nullptr;
}

bool UI::Initialize(HWND hwnd)
{
  util::log::Write("Initializing UI");

  m_pSwapChain = FC::GetSwapChain();
  m_pDevice = FC::GetDevice();
  if (!m_pDevice)
  {
    util::log::Error("Could not get ID3D11Device");
    return false;
  }

  m_pDevice->GetImmediateContext(&m_pContext);
  if (!m_pContext)
  {
    util::log::Error("Could not get ID3D11DeviceContext");
    return false;
  }

  bool result = ImGui_ImplDX11_Init(hwnd, m_pDevice, m_pContext);
  if (!result)
  {
    util::log::Error("ImGui_ImplDX11_Init failed");
    return false;
  }

  DWORD thread_id = GetWindowThreadProcessId(hwnd, NULL);
  if (thread_id)
  {
    if (!(g_getMessageHook = SetWindowsHookEx(WH_GETMESSAGE, this->GetMessage_Callback, g_dllHandle, thread_id)))
    {
      util::log::Error("Couldn't create WH_GETMESSAGE hook. GetLastError 0x%X", GetLastError());
      return false;
    }
    if (!(g_callWndProcHook = SetWindowsHookEx(WH_CALLWNDPROC, this->CallWndProc_Callback, g_dllHandle, thread_id)))
    {
      util::log::Error("Couldn't create WH_GETMESSAGE hook. GetLastError 0x%X", GetLastError());
      return false;
    }
  }
  else
    util::log::Warning("Could not find window thread handle");

  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontDefault();

  ImFontConfig config;
  config.OversampleH = 8;
  config.OversampleV = 8;
  config.FontDataOwnedByAtlas = false;

  ImFontConfig configOffset = config;
  configOffset.GlyphOffset = ImVec2(0, -2);

  void* pData;
  DWORD dataSize;

  if (util::GetResource(IDR_FONT_PURISTA, pData, dataSize))
  {
    io.Fonts->AddFontFromMemoryTTF(pData, dataSize, 24, &config);
    io.Fonts->AddFontFromMemoryTTF(pData, dataSize, 20, &configOffset);
    io.Fonts->AddFontFromMemoryTTF(pData, dataSize, 18, &configOffset);
  }
  else
  {
    util::log::Error("Failed to load IDR_FONT_PURISTASEMI from resources");
    return false;
  }

  if (util::GetResource(IDR_FONT_SEGOE, pData, dataSize))
  {
    io.Fonts->AddFontFromMemoryTTF(pData, dataSize, 16, &config);
    io.Fonts->AddFontFromMemoryTTF(pData, dataSize, 20, &config);
  }
  else
  {
    util::log::Error("Failed to load IDR_FONT_SEGOEUI from resources");
    return false;
  }

  util::log::Write("Loading image resources");
  for (int i = IDR_IMG_BG1; i <= IDR_IMG_BG1; ++i)
    m_bgImages.emplace_back(CreateImageFromResource(i));

  //m_updateBg = pRenderer->CreateImageFromResource(IDR_BG_UPDATE);
  m_titleImg = CreateImageFromResource(IDR_IMG_TITLE);

  //srand(time(NULL));
  //m_bgIndex = rand() % m_bgImages.size();
  //m_nextIndex = m_bgIndex;
  //while (m_bgIndex == m_nextIndex)
  //  m_nextIndex = rand() % m_bgImages.size();

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

  if (!CreateRenderTarget())
    return false;

  m_cursor = LoadCursor(NULL, IDC_ARROW);
  util::log::Write("Cursor 0x%X", m_cursor);
  if (!m_cursor)
  {
    m_cursor = LoadCursor(FC::FCHandle, IDC_ARROW);
    util::log::Write("Cursor 0x%X", m_cursor);
  }

  m_initialized = true;
  util::log::Ok("UI initialized");

  Toggle();
  return true;
}

bool UI::CreateRenderTarget()
{
  util::log::Write("CreateRenderTarget");
  m_pDevice = FC::GetDevice();
  m_pDevice->GetImmediateContext(&m_pContext);

  IDXGISwapChain* pSwapChain = FC::GetSwapChain();
  if (!pSwapChain)
  {
    util::log::Error("Could not get IDXGISwapChain");
    return false;
  }

  ID3D11Texture2D* pBackBuffer = nullptr;
  pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
  if (!pBackBuffer)
  {
    util::log::Error("Could not retrieve backbuffer from IDXGISwapChain");
    return false;
  }

  HRESULT hr = m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRTV);
  if (FAILED(hr))
  {
    util::log::Error("CreateRenderTargetView failed");
    return false;
  }

  pBackBuffer->Release();
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

  HRESULT hr = DirectX::CreateWICTextureFromMemory(m_pDevice, (const uint8_t*)pData, szData, (ID3D11Resource**)&newImg.pTexture, &newImg.pSRV);
  if (FAILED(hr))
    util::log::Error("CreateWICTextureFromMemory failed");

  return newImg;
}

void UI::ResizeBuffers(bool release /*= false*/)
{
  m_framesToSkip = 1000;
  m_isResizing = true;
  if (m_pRTV && release)
  {
    m_pRTV->Release();
    m_pRTV = nullptr;
    ImGui_ImplDX11_InvalidateDeviceObjects();
    util::log::Write("Buffer resize");
  }
}

void UI::Toggle()
{
  m_enabled = !m_enabled;

}

void UI::Draw()
{
  if (!m_initialized) return;
  if (m_isResizing)
  {
    m_framesToSkip -= 1;
    if (m_framesToSkip > 0)
      return;

    CreateRenderTarget();
    m_isResizing = false;
    return;
  }

  ImGuiIO& io = ImGui::GetIO();
  m_pContext->OMSetRenderTargets(1, &m_pRTV, NULL);

  ImGui_ImplDX11_NewFrame();
  if (m_enabled)
  {
    ImGui::SetNextWindowSize(ImVec2(800, 460));
    ImGui::Begin("Far Cry 5 Cinematic Tools", nullptr, ImGuiWindowFlags_NoResize);
    {
      ImVec2 windowPos = ImGui::GetWindowPos();
      ImGui::Dummy(ImVec2(0, 20));

      ImGui::GetWindowDrawList()->AddImage(m_bgImages[0].pSRV, ImVec2(windowPos.x, windowPos.y + 19), ImVec2(windowPos.x + 800, windowPos.y + 460));
      //if (m_isFading)
      //  ImGui::GetWindowDrawList()->AddImage(m_bgImages[m_nextIndex].pShaderResourceView.Get(), ImVec2(windowPos.x, windowPos.y + 19), ImVec2(windowPos.x + 800, windowPos.y + 460),
      //    ImVec2(0, 0), ImVec2(1, 1), ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, m_opacity)));

      ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(windowPos.x, windowPos.y + 19), ImVec2(windowPos.x + 800, windowPos.y + 460), 0x10000000);
      ImGui::GetWindowDrawList()->AddImage(m_titleImg.pSRV, ImVec2(windowPos.x + 210, windowPos.y + 85), ImVec2(windowPos.x + 592, windowPos.y + 131));

      ImGui::PushFont(io.Fonts->Fonts[2]);
      ImGui::Dummy(ImVec2(143, 33));
      ImGui::SameLine(0, 0);

      if (ImGui::ToggleButton("CAMERA", ImVec2(158, 33), eSelectedMenu == UIMenu_Camera, false))
        eSelectedMenu = UIMenu_Camera;

      ImGui::SameLine(0, 20);
      if (ImGui::ToggleButton("ENVIRONMENT", ImVec2(158, 33), eSelectedMenu == UIMenu_Visuals, false))
        eSelectedMenu = UIMenu_Visuals;

      ImGui::SameLine(0, 20);
      if (ImGui::ToggleButton("MISC", ImVec2(158, 33), eSelectedMenu == UIMenu_Misc, false))
        eSelectedMenu = UIMenu_Misc;

      ImGui::PopFont();

      ImGui::GetWindowDrawList()->AddLine(ImVec2(windowPos.x + 143, windowPos.y + 76), ImVec2(windowPos.x + 301, windowPos.y + 76), 0xFF1C79E5, 2);
      ImGui::GetWindowDrawList()->AddLine(ImVec2(windowPos.x + 321, windowPos.y + 76), ImVec2(windowPos.x + 479, windowPos.y + 76), 0xFF1C79E5, 2);
      ImGui::GetWindowDrawList()->AddLine(ImVec2(windowPos.x + 499, windowPos.y + 76), ImVec2(windowPos.x + 657, windowPos.y + 76), 0xFF1C79E5, 2);

      ImGui::Dummy(ImVec2(0, 50));
      ImGui::Dummy(ImVec2(0, 0));
      ImGui::SameLine(10);

      {
        ImGui::BeginChild("contentChild", ImVec2(-10, -10), false);

        if (eSelectedMenu == UIMenu_Camera)
          g_mainHandle->GetCameraManager()->DrawUI();
        else if (eSelectedMenu == UIMenu_Visuals)
        {
          g_mainHandle->GetEnvironmentManager()->DrawUI();
        }
        else if (eSelectedMenu == UIMenu_Misc)
        {
          ImGui::Columns(3, "miscColumns", false);
          ImGui::NextColumn();
          ImGui::SetColumnOffset(-1, 5);

          ImGui::PushFont(io.Fonts->Fonts[4]);
          ImGui::PushItemWidth(130);

          FC::CAIKnowledge* pAIKnowledge = FC::CAIKnowledge::Singleton();
          if (pAIKnowledge)
            ImGui::Checkbox("Disable AI", (bool*)&pAIKnowledge->m_ZombieAI);

          ImGui::Text("Timescale");
          if (ImGui::InputFloat("##GameTimescale", &g_timeScale, 0.1, 0.1, 3))
          {
            if (g_timeScale < 0.001)
              g_timeScale = 0.001;

            FC::CTimer* pTimer = FC::CTimer::Singleton();
            if (pTimer)
              pTimer->m_TimeScale = g_timeScale;
          }

          /*ImGui::PushFont(io.Fonts->Fonts[3]);
          ImGui::Dummy(ImVec2(0, 10));

          ImGui::DrawWithBorders([=]
          {
            if (ImGui::Button("Config", ImVec2(158, 33)))
            {
              // TODO: Show config menu
            }
          });

          ImGui::PopFont();*/
          ImGui::NextColumn();
          ImGui::SetColumnOffset(-1, 388.5f);

          ImGui::PushFont(io.Fonts->Fonts[2]);
          ImGui::Dummy(ImVec2(0, 5));
          ImGui::Text("CREDITS"); ImGui::PopFont();

          ImGui::TextWrapped(g_creditsText);

          ImGui::PopFont();
        }
        ImGui::EndChild();
      }

    } ImGui::End();

    //g_mainHandle->GetChromaTool()->DrawUI();
    //g_mainHandle->GetConfig()->DrawUI();
  }
  ImGui::Render();

  m_hasKeyboardFocus = ImGui::GetIO().WantCaptureKeyboard;
  m_hasMouseFocus = ImGui::GetIO().WantCaptureMouse;

  m_mousePos.x = ImGui::GetIO().MousePos.x;
  m_mousePos.y = ImGui::GetIO().MousePos.y;

  m_pContext->OMSetRenderTargets(0, NULL, NULL);
}

void UI::Update(double dt)
{
  if (m_isResizing)
  {
    IDXGISwapChain* pSwapChain = FC::GetSwapChain();
    if (pSwapChain == m_pSwapChain)
    {
      m_swapchainTimeout += dt;
      if (m_swapchainTimeout > 10.0)
      {
        m_swapchainTimeout = 0;
        util::log::Write("Timed out while waiting for new SwapChain (No actual resize took place?)");
        util::hooks::EnableHooks();
      }
      return;
    }

    m_resizeTimer += dt;
    if (m_resizeTimer < 2.0)
      return;

    m_resizeTimer = 0;
    util::log::Write("New swapchain detected");
    m_pSwapChain = pSwapChain;
    util::hooks::EnableHooks();
  }

  if (!m_enabled) return;

  m_dtFade += dt;
  if ((m_isFading = m_dtFade > 60))
  {
    m_opacity = (m_dtFade - 60.f) / 5.0f;
    if (m_dtFade - 60.f >= 5.0f)
    {
      m_bgIndex = m_nextIndex;
      m_isFading = false;

      //while (m_bgIndex == m_nextIndex)
        //m_nextIndex = rand() % m_bgImages.size();

      m_opacity = 0;
      m_dtFade = 0;
    }
  }
}

void UI::Release()
{
  ImGui_ImplDX11_Shutdown();

  UnhookWindowsHookEx(g_getMessageHook);
  UnhookWindowsHookEx(g_callWndProcHook);
  PDWORD_PTR dwResult = 0;
  SendMessageTimeout(HWND_BROADCAST, WM_NULL, 0, 0, SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG, 1000, dwResult);

  g_getMessageHook = NULL;
}

UI::~UI()
{
}

LRESULT __stdcall UI::CallWndProc_Callback(int nCode, WPARAM wParam, LPARAM lParam)
{
  CWPSTRUCT* pMsg = (CWPSTRUCT*)lParam;
  switch (pMsg->message)
  {
  case WM_ENTERSIZEMOVE:
  case WM_MOVE:
    if (!g_mainHandle->GetUI()->m_isResizing)
    {
      util::hooks::DisableHooks();
      g_mainHandle->GetUI()->ResizeBuffers(true);
    }
    break;
  case WM_EXITSIZEMOVE:
    break;
  }

  return CallNextHookEx(g_callWndProcHook, nCode, wParam, lParam);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall UI::GetMessage_Callback(int nCode, WPARAM wParam, LPARAM lParam)
{
  if (!g_mainHandle->GetUI()->m_isResizing)
  {
    MSG* pMsg = (MSG*)lParam;
    if (pMsg->message == WM_KEYDOWN)
      return CallNextHookEx(g_getMessageHook, nCode, wParam, lParam);

    if (!ImGui_ImplWin32_WndProcHandler(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam))
    {
      if (pMsg->message == WM_MOUSEMOVE)
        g_mainHandle->GetInputManager()->HandleInputMessage(pMsg->lParam);
    }
  }

  return CallNextHookEx(g_getMessageHook, nCode, wParam, lParam);
}
