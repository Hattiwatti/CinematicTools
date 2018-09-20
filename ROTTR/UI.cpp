#include "UI.h"
#include "Globals.h"
#include "Util/Util.h"
#include "Util/ImGuiEXT.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "resource.h"

#include <winerror.h>
#include <WICTextureLoader.h>
#pragma comment(lib, "DirectXTK.lib")

const char* g_creditsText = "The following libraries are used in making the Cinematic Tools:\n"
"MinHook by Tsuda Kageyu, inih by Ben Hoyt, ImGui by Omar Cornut, C++ libraries by Boost, DirectXTK and DirectXTex by Microsoft.\n\n"
"Thank you for using the tools and making awesome stuff with it!\n\n"
"Remember to report bugs at www.cinetools.xyz/bugs";

UI::UI() :
  m_Enabled(false),
  m_FramesToSkip(0),
  m_HasKeyboardFocus(false),
  m_HasMouseFocus(false)
{

}

UI::~UI()
{
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
}

bool UI::Initialize()
{
  /////////////////////////
  // ImGui Configuration //
  /////////////////////////


  ImGui::CreateContext();
  ImGui_ImplWin32_Init(g_gameHwnd);

  if (!ImGui_ImplDX11_Init(g_d3d11Device, g_d3d11Context))
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

  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontDefault();

  ImFontConfig fontConfig;
  fontConfig.OversampleH = 8;
  fontConfig.OversampleV = 8;
  fontConfig.FontDataOwnedByAtlas = false;

  ImFontConfig fontOffsetConfig = fontConfig;
  fontOffsetConfig.GlyphOffset = ImVec2(0, -2);

  void* pData;
  DWORD szData;

  if (!util::GetResource(IDR_FONT_PURISTA, pData, szData))
  {
    util::log::Error("Failed to load IDR_FONT_PURISTASEMI from resources");
    return false;
  }

  io.Fonts->AddFontFromMemoryTTF(pData, szData, 24, &fontConfig);
  io.Fonts->AddFontFromMemoryTTF(pData, szData, 20, &fontOffsetConfig);
  io.Fonts->AddFontFromMemoryTTF(pData, szData, 18, &fontOffsetConfig);

  if (!util::GetResource(IDR_FONT_SEGOE, pData, szData))
  {
    util::log::Error("Failed to load IDR_FONT_SEGOEUI from resources");
    return false;
  }

  io.Fonts->AddFontFromMemoryTTF(pData, szData, 16, &fontConfig);
  io.Fonts->AddFontFromMemoryTTF(pData, szData, 20, &fontConfig);


  //////////////////////////////////////
  // Background & title image loading //
  //////////////////////////////////////

  m_TitleImage = g_mainHandle->GetRenderer()->CreateImageFromResource(IDR_IMG_TITLE);
  for (int i = IDR_IMG_BG1; i <= IDR_IMG_BG1; ++i)
    m_BgImages.emplace_back(g_mainHandle->GetRenderer()->CreateImageFromResource(i));

  util::log::Ok("UI Initialized");
  UI::Toggle();
  return true;
}

void UI::Draw()
{
  if (!m_Enabled) return;

  ImGuiIO& io = ImGui::GetIO();
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  ImGui::SetNextWindowSize(ImVec2(800, 470));
  ImGui::Begin("Cinematic Tools", nullptr, ImGuiWindowFlags_NoResize);
  {
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImGui::Dummy(ImVec2(0, 20));

    ImGui::GetWindowDrawList()->AddImage(m_BgImages[0].pSRV.Get(), ImVec2(windowPos.x, windowPos.y + 19), ImVec2(windowPos.x + 800, windowPos.y + 460));
    //if (m_isFading)
    //  ImGui::GetWindowDrawList()->AddImage(m_bgImages[m_nextIndex].pShaderResourceView.Get(), ImVec2(windowPos.x, windowPos.y + 19), ImVec2(windowPos.x + 800, windowPos.y + 460),
    //    ImVec2(0, 0), ImVec2(1, 1), ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, m_opacity)));

    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(windowPos.x, windowPos.y + 19), ImVec2(windowPos.x + 800, windowPos.y + 460), 0x10000000);
    ImGui::GetWindowDrawList()->AddImage(m_TitleImage.pSRV.Get(), ImVec2(windowPos.x + 210, windowPos.y + 85), ImVec2(windowPos.x + 592, windowPos.y + 131));

    ImGui::PushFont(io.Fonts->Fonts[2]);
    ImGui::Dummy(ImVec2(50, 35));
    ImGui::SameLine(0, 0);

    if (ImGui::ToggleButton("CAMERA", ImVec2(160, 35), m_SelectedMenu == SelectedMenu::Camera, false))
      m_SelectedMenu = SelectedMenu::Camera;

    ImGui::SameLine(0, 20);
    if (ImGui::Button("VISUALS", ImVec2(160, 35)))
      ImGui::OpenPopup("visualMenus");

    ImVec2 popupVisualsPos(windowPos.x + 230, windowPos.y + 78);
    if (ImGui::IsPopupOpen("visualMenus"))
      ImGui::SetNextWindowPos(popupVisualsPos);

    if (ImGui::BeginPopup("visualMenus"))
    {
      if (ImGui::ToggleButton("Camera", ImVec2(158, 35), m_SelectedMenu == SelectedMenu::Visuals_Camera, false))
      {
        m_SelectedMenu = SelectedMenu::Visuals_Camera;
        ImGui::CloseCurrentPopup();
      }

      if (ImGui::ToggleButton("Depth of Field", ImVec2(160, 35), m_SelectedMenu == SelectedMenu::Visuals_DOF, false))
      {
        m_SelectedMenu = SelectedMenu::Visuals_DOF;
        ImGui::CloseCurrentPopup();
      }

      ImGui::Dummy(ImVec2(0, 0));
      ImGui::EndPopup();
    }

    ImGui::SameLine(0, 20);
    if (ImGui::ToggleButton("SCENE", ImVec2(160, 35), m_SelectedMenu == SelectedMenu::Scene, false))
      m_SelectedMenu = SelectedMenu::Scene;

    ImGui::SameLine(0, 20);
    if (ImGui::ToggleButton("MISC", ImVec2(160, 35), m_SelectedMenu == SelectedMenu::Misc, false))
      m_SelectedMenu = SelectedMenu::Misc;

    ImGui::PopFont();

    ImGui::GetWindowDrawList()->AddLine(ImVec2(windowPos.x + 50, windowPos.y + 78), ImVec2(windowPos.x + 210, windowPos.y + 78), 0xFF1C79E5, 2);
    ImGui::GetWindowDrawList()->AddLine(ImVec2(windowPos.x + 230, windowPos.y + 78), ImVec2(windowPos.x + 390, windowPos.y + 78), 0xFF1C79E5, 2);
    ImGui::GetWindowDrawList()->AddLine(ImVec2(windowPos.x + 410, windowPos.y + 78), ImVec2(windowPos.x + 570, windowPos.y + 78), 0xFF1C79E5, 2);
    ImGui::GetWindowDrawList()->AddLine(ImVec2(windowPos.x + 590, windowPos.y + 78), ImVec2(windowPos.x + 750, windowPos.y + 78), 0xFF1C79E5, 2);

    ImGui::Dummy(ImVec2(0, 50));
    ImGui::Dummy(ImVec2(0, 0));
    ImGui::SameLine(10);

    {
      ImGui::BeginChild("contentChild", ImVec2(-10, -10), false);

      if (m_SelectedMenu == SelectedMenu::Camera)
      {
        //g_mainHandle->GetCameraManager()->DrawUI();
      }
      else if (m_SelectedMenu >= SelectedMenu::Visuals_Camera && m_SelectedMenu <= SelectedMenu::Visuals_DOF)
      {
        //g_mainHandle->GetVisualEnvManager()->DrawUI(m_SelectedMenu);
      }
      else if (m_SelectedMenu == SelectedMenu::Scene)
      {
        //g_mainHandle->GetSceneEditor()->DrawUI();
      }
      else if (m_SelectedMenu == SelectedMenu::Misc)
      {
        ImGui::Columns(3, "miscColumns", false);
        ImGui::NextColumn();
        ImGui::SetColumnOffset(-1, 5);

        ImGui::PushFont(io.Fonts->Fonts[4]);
        ImGui::PushItemWidth(130);

        ImGui::PushFont(io.Fonts->Fonts[3]);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 1, 1, 1));
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::DrawWithBorders([=]
        {
          if (ImGui::Button("Config", ImVec2(158, 33)))
            g_mainHandle->GetInputSystem()->ShowUI();
          //if (ImGui::Button("Objects", ImVec2(158, 33)))
          //  g_mainHandle->GetObjectInteractor()->ShowUI();
        });

        ImGui::PopStyleColor();
        ImGui::PopFont();

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

  //g_mainHandle->GetObjectInteractor()->DrawUI();
  g_mainHandle->GetInputSystem()->DrawUI();

  ImGui::Render();
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

  m_HasKeyboardFocus = ImGui::GetIO().WantCaptureKeyboard;
  m_HasMouseFocus = ImGui::GetIO().WantCaptureMouse;
}

void UI::Update(double dt)
{
  // For fancy background fading effects
}

void UI::Toggle()
{
  m_Enabled = !m_Enabled;
}
