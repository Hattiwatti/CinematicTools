#include "UI.h"
#include "Main.h"
#include "resource.h"
#include "ImGui/imgui_impl_dx11.h"
#include "Util/Util.h"

#include <WICTextureLoader.h>
#include "Util/ImGuiHelpers.h"
#pragma comment(lib, "DirectXTK.lib")

HHOOK g_getMessageHook = 0;

UI::UI()
{
  m_initialized = false;

  m_enabled = true;
  m_hasKeyboardFocus = false;
  m_hasMouseFocus = false;
  eSelectedMenu = UIMenu_Camera;
  m_dtFade = 0;

  m_pRenderer = nullptr;
}

bool UI::Initialize(RendererImpl* pRenderer, HWND hwnd)
{
  util::log::Write("Initializing UI");

  m_pRenderer = pRenderer;

  DWORD thread_id = GetWindowThreadProcessId(hwnd, NULL);
  if (thread_id)
  {
    if (!(g_getMessageHook = SetWindowsHookEx(WH_GETMESSAGE, this->GetMessage_Callback, g_dllHandle, thread_id)))
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

  if (util::GetResource(IDR_FONT_SEGOEUI, pData, dataSize))
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
  //for (int i = IDR_BGIMAGE1; i <= IDR_BGIMAGE5; ++i)
  //{
  // ImageWrapper* bgNew = pRenderer->CreateImageFromResource(i);
  // m_bgImages.push_back(bgNew);s
  //}

  //m_updateBg = pRenderer->CreateImageFromResource(IDR_BG_UPDATE);
  m_titleImg = pRenderer->CreateImageFromResource(IDR_IMG_TITLE);

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

  m_initialized = true;
  util::log::Ok("UI initialized");

  return true;
}

void UI::Toggle()
{
  m_enabled = !m_enabled;
}

void UI::Draw()
{
  if (!m_initialized) return;

  ImGuiIO& io = ImGui::GetIO();

  m_pRenderer->ImGui_BeginFrame();
  if (m_enabled)
  {
    ImGui::SetNextWindowSize(ImVec2(800, 460));
    ImGui::Begin("Quantum Break Cinematic Tools", nullptr, ImGuiWindowFlags_NoResize);
    {
      ImVec2 windowPos = ImGui::GetWindowPos();
      ImGui::Dummy(ImVec2(0, 20));

      ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(windowPos.x, windowPos.y + 19), ImVec2(windowPos.x + 800, windowPos.y + 460), 0x10000000);
      ImGui::GetWindowDrawList()->AddImage(m_titleImg->GetView(), ImVec2(windowPos.x + 210, windowPos.y + 85), ImVec2(windowPos.x + 592, windowPos.y + 131));

      ImGui::PushFont(io.Fonts->Fonts[2]);
      ImGui::Dummy(ImVec2(143, 33));
      ImGui::SameLine(0, 0);

      if (ImGui::ToggleButton("CAMERA", ImVec2(158, 33), eSelectedMenu == UIMenu_Camera, false))
        eSelectedMenu = UIMenu_Camera;

      ImGui::SameLine(0, 20);
      if (ImGui::ToggleButton("VISUALS", ImVec2(158, 33), eSelectedMenu == UIMenu_Visuals, false))
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
        ImGui::EndChild();
      }

    } ImGui::End();
  }
  ImGui::Render();

  m_hasKeyboardFocus = ImGui::GetIO().WantCaptureKeyboard;
  m_hasMouseFocus = ImGui::GetIO().WantCaptureMouse;

  m_mousePos.x = ImGui::GetIO().MousePos.x;
  m_mousePos.y = ImGui::GetIO().MousePos.y;
}

void UI::Update(double dt)
{
  if (!m_enabled) return;

  m_dtFade += dt;
  if ((m_isFading = m_dtFade > 60))
  {
    m_opacity = (m_dtFade - 60.f) / 5.0f;
    if (m_dtFade - 60.f >= 5.0f)
    {
      m_bgIndex = m_nextIndex;
      m_isFading = false;

      while (m_bgIndex == m_nextIndex)
        m_nextIndex = rand() % m_bgImages.size();

      m_opacity = 0;
      m_dtFade = 0;
    }
  }
}

void UI::Release()
{
  ImGui_ImplDX11_Shutdown();

  UnhookWindowsHookEx(g_getMessageHook);
  PDWORD_PTR dwResult = 0;
  SendMessageTimeout(HWND_BROADCAST, WM_NULL, 0, 0, SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG, 1000, dwResult);

  g_getMessageHook = NULL;
}

UI::~UI()
{
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall UI::GetMessage_Callback(int nCode, WPARAM wParam, LPARAM lParam)
{
  MSG* pMsg = (MSG*)lParam;
  if (pMsg->message == WM_KEYDOWN)
  {
    return CallNextHookEx(g_getMessageHook, nCode, wParam, lParam);
  }

  if (!ImGui_ImplWin32_WndProcHandler(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam))
  {
  }

  return CallNextHookEx(g_getMessageHook, nCode, wParam, lParam);
}