#include "UI.h"
#include "Main.h"
#include "Util/Util.h"
#include "Util/ImGuiEXT.h"
#include "resource.h"
#include "imgui/imgui_impl_dx11.h"

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
  m_HasMouseFocus(false),
  m_IsResizing(false),
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
  m_pCommonStates = std::make_unique<DirectX::CommonStates>(g_d3d11Device);
  m_hCursor = LoadCursor(NULL, IDC_ARROW);

  /////////////////////////
  // ImGui Configuration //
  /////////////////////////

  // ImGui is an immensely useful and simple-to-use UI system made by Omar Cornut.
  // Definitely check it out:
  // https://github.com/ocornut/imgui

  util::log::Write("Initializing ImGui");

  ImGui::CreateContext();
  if (!ImGui_ImplDX11_Init(g_gameHwnd, g_d3d11Device, g_d3d11Context))
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
  Style.Colors[ImGuiCol_FrameBg] = ImVec4(.2f, .2f, .2f, 1.0f);
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

  util::log::Write("Loading UI resources");

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

  m_TitleImage = CreateImageFromResource(IDR_IMG_TITLE);
  for (int i = IDR_IMG_BG1; i <= IDR_IMG_BG1; ++i)
    m_BgImages.emplace_back(CreateImageFromResource(i));

  if (!CreateRenderTarget())
    return false;

  util::log::Ok("UI Initialized");
  m_Enabled = true;
  return true;
}

void UI::Draw()
{
  if (!m_Enabled) return;
  if (m_IsResizing)
  {
    // I think it's a good idea to skip some frames after a resize
    // juuuuust to make sure the game's done with resizing stuff.

    m_FramesToSkip -= 1;
    if (m_FramesToSkip == 0)
    {
      m_IsResizing = false;
      CreateRenderTarget();
    }
    return;
  }

  g_d3d11Context->PSSetShader(nullptr, nullptr, 0);
  g_d3d11Context->GSSetShader(nullptr, nullptr, 0);
  g_d3d11Context->VSSetShader(nullptr, nullptr, 0);

  //g_d3d11Context->ClearRenderTargetView(m_pRTV.Get(), clearColor);
  g_d3d11Context->OMSetRenderTargets(1, m_pRTV.GetAddressOf(), nullptr);
  g_d3d11Context->OMSetDepthStencilState(m_pCommonStates->DepthNone(), 0);
  g_d3d11Context->OMSetBlendState(m_pCommonStates->Opaque(), nullptr, 0xFFFFFFFF);
  g_d3d11Context->RSSetState(m_pCommonStates->CullNone());

  ImGuiIO& io = ImGui::GetIO();
  ImGui_ImplDX11_NewFrame();

  ImGui::SetNextWindowSize(ImVec2(800, 460));
  ImGui::Begin("theHunter: Call of the Wild Cinematic Tools", nullptr, ImGuiWindowFlags_NoResize);
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
    ImGui::Dummy(ImVec2(220, 33));
    ImGui::SameLine(0, 0);

    if (ImGui::ToggleButton("CAMERA", ImVec2(160, 33), m_SelectedMenu == UIMenu_Camera, false))
      m_SelectedMenu = UIMenu_Camera;

    ImGui::SameLine(0, 20);
    if (ImGui::ToggleButton("MISC", ImVec2(160, 33), m_SelectedMenu == UIMenu_Misc, false))
      m_SelectedMenu = UIMenu_Misc;

    ImGui::PopFont();

    ImGui::GetWindowDrawList()->AddLine(ImVec2(windowPos.x + 220, windowPos.y + 76), ImVec2(windowPos.x + 380, windowPos.y + 76), 0xFF1C79E5, 2);
    ImGui::GetWindowDrawList()->AddLine(ImVec2(windowPos.x + 400, windowPos.y + 76), ImVec2(windowPos.x + 560, windowPos.y + 76), 0xFF1C79E5, 2);

    ImGui::Dummy(ImVec2(0, 50));
    ImGui::Dummy(ImVec2(0, 0));
    ImGui::SameLine(10);

    {
      ImGui::BeginChild("contentChild", ImVec2(-10, -10), false);

      if (m_SelectedMenu == UIMenu_Camera)
        g_mainHandle->GetCameraManager()->DrawUI();
      else if (m_SelectedMenu == UIMenu_Visuals)
      {
      }
      else if (m_SelectedMenu == UIMenu_Misc)
      {
        ImGui::Columns(3, "miscColumns", false);
        ImGui::NextColumn();
        ImGui::SetColumnOffset(-1, 5);

        ImGui::PushFont(io.Fonts->Fonts[4]);
        ImGui::PushItemWidth(130);

        ImGui::DrawWithBorders([=]
        {
          if (ImGui::Button("Config", ImVec2(158, 33)))
            g_mainHandle->GetInputSystem()->ShowUI();
        });

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

  g_mainHandle->GetInputSystem()->DrawUI();

  ImGui::Render();

  m_HasKeyboardFocus = ImGui::GetIO().WantCaptureKeyboard;
  m_HasMouseFocus = ImGui::GetIO().WantCaptureMouse;

  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void UI::OnResize()
{
  // Backbuffer needs to be resized when the window size/resolution changes.
  // However it's bound to the UI's RenderTarget (CreateRenderTarget()), so
  // it needs to be released before the backbuffer can be resized.
  // Failing to handle this will usually result in a crash.

  m_IsResizing = true;
  m_FramesToSkip = 100;
  if (m_pRTV)
    m_pRTV.Reset();
}

void UI::Update(double dt)
{
  // For fancy background fading effects
}

void UI::Toggle()
{
  m_Enabled = !m_Enabled;
  if (m_Enabled)
    SetCursor(m_hCursor);
}

bool UI::CreateRenderTarget()
{
  util::log::Write("Creating a render target view");

  // Creates a render target to backbuffer resource
  // Should guarantee that stuff actually gets drawn
  ID3D11Texture2D* pBackBuffer = nullptr;
  HRESULT hr = g_dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
  if (FAILED(hr) || !pBackBuffer)
  {
    util::log::Error("Failed to retrieve backbuffer from SwapChain, HRESULT 0x%X", hr);
    return false;
  }

  hr = g_d3d11Device->CreateRenderTargetView(pBackBuffer, NULL, m_pRTV.ReleaseAndGetAddressOf());
  pBackBuffer->Release();
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

  HRESULT hr = DirectX::CreateWICTextureFromMemory(g_d3d11Device, (const uint8_t*)pData, szData, 
                (ID3D11Resource**)newImg.pTexture.ReleaseAndGetAddressOf(), newImg.pSRV.ReleaseAndGetAddressOf());
  if (FAILED(hr))
    util::log::Error("CreateWICTextureFromMemory failed");

  return newImg;
}