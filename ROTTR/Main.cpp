#include "Main.h"
#include "Globals.h"
#include "Util/Util.h"
#include "Foundation.h"

#include <boost/chrono.hpp>
#include <boost/filesystem.hpp>

static const char* g_gameName = "Rise of the Tomb Raider";
static const char* g_moduleName = "ROTTR.exe";
static const char* g_className = "TR2NxApp";
static const char* g_configFile = "./Cinematic Tools/config.ini";

Main* g_mainHandle = nullptr;
HINSTANCE g_dllHandle = NULL;

HINSTANCE g_gameHandle = NULL;
HWND g_gameHwnd = NULL;

WNDPROC g_origWndProc = 0;
bool g_shutdown = false;
bool g_hasFocus = true;

ID3D11DeviceContext* g_d3d11Context = nullptr;
ID3D11Device* g_d3d11Device = nullptr;
IDXGISwapChain* g_dxgiSwapChain = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

Main::Main() : 
  m_Initialized(false),
  m_ConfigChanged(false),
  m_dtConfigCheck(0)
{

}

Main::~Main()
{

}

bool Main::Initialize()
{
  boost::filesystem::path dir("Cinematic Tools");
  if (!boost::filesystem::exists(dir))
    boost::filesystem::create_directory(dir);

  util::log::Init();
  util::log::Write("Cinematic Tools for %s\n", g_gameName);

  g_gameHwnd = FindWindowA(g_className, NULL);
  if (g_gameHwnd == NULL)
  {
    util::log::Error("Failed to retrieve window handle, GetLastError 0x%X", GetLastError());
    return false;
  }

  // Used for relative offsets
  g_gameHandle = GetModuleHandleA(g_moduleName);
  if (g_gameHandle == NULL)
  {
    util::log::Error("Failed to retrieve module handle, GetLastError 0x%X", GetLastError());
    return false;
  }

  g_d3d11Device = Foundation::PCDX11DeviceManager::Singleton()->m_pD3D11Device; // Fetch ID3D11Device
  if (g_d3d11Device)
    g_d3d11Device->GetImmediateContext(&g_d3d11Context);

  g_dxgiSwapChain = Foundation::PCDX11RenderDevice::Singleton()->m_Dx11SwapChain->m_SwapChain; // Fetch SwapChain

  if (!g_d3d11Context || !g_d3d11Device || !g_dxgiSwapChain)
  {
    util::log::Error("Failed to retrieve Dx11 interfaces");
    util::log::Error("Device 0x%I64X DeviceContext 0x%I64X SwapChain 0x%I64X", g_d3d11Device, g_d3d11Context, g_dxgiSwapChain);
    return false;
  }

  m_Renderer = std::make_unique<CTRenderer>();
  if (!m_Renderer->Initialize())
    return false;

  m_CameraManager = std::make_unique<CameraManager>();
  m_InputSystem = std::make_unique<InputSystem>();
  m_UI = std::make_unique<UI>();

  m_InputSystem->Initialize();
  if (!m_UI->Initialize())
    return false;

  util::hooks::Init();
  LoadConfig();

  // Subclass the window with a new WndProc to catch messages
  g_origWndProc = (WNDPROC)SetWindowLongPtr(g_gameHwnd, -4, (LONG_PTR)&WndProc);
  if (g_origWndProc == 0)
  {
    util::log::Error("Failed to set WndProc, GetLastError 0x%X", GetLastError());
    return false;
  }

  m_Initialized = true;
  return true;
}

// Foundation::SceneLight* pTestLight = nullptr;
// Foundation::SceneEntityData* pEntityData = nullptr;
// Foundation::CommonLightResource* pResource = nullptr;

void Main::Run()
{
  boost::chrono::high_resolution_clock::time_point lastUpdate = boost::chrono::high_resolution_clock::now();
  while (!g_shutdown)
  {
    boost::chrono::duration<double> dt = boost::chrono::high_resolution_clock::now() - lastUpdate;
    lastUpdate = boost::chrono::high_resolution_clock::now();

//     if (GetKeyState(VK_F8) & 0x8000)
//     {
//       if (!pResource)
//       {
//         pResource = Foundation::CommonLightResource::Create();
//         //pResource->LoadDefaultValues();
//         //pResource->m_Color = { 1, 1, 1, 1 };
//         pResource = (Foundation::CommonLightResource*)0x93B236E0;
//       }
// 
//       if (!pEntityData)
//         pEntityData = new Foundation::SceneEntityData();
// 
//       util::log::Write("CommonLightResource 0x%I64X", pResource);
//       util::log::Write("SceneEntityData 0x%I64X", pEntityData);
//       Foundation::SceneLight* pNewLight = Foundation::Scene::Singleton()->CreateLight(pResource);
// 
//       util::log::Write("SceneLight 0x%I64X", pNewLight);
//       pNewLight->SetEntityData(pEntityData);
// 
//       //pNewLight->m_CommonRenderLight->LoadDefaultValues();
//       pNewLight->m_CommonRenderLight->m_SceneLight = pNewLight;
//       pNewLight->m_CommonRenderLight->m_LightResource = pResource;
//       pNewLight->m_CommonRenderLight->m_AttenuationDistance = 200.f;
// 
//       Foundation::GameRender* pGameRender = Foundation::GameRender::Singleton();
//       pNewLight->m_CommonRenderLight->m_Transform = pGameRender->m_CameraTransform;
// 
//       util::log::Write("CommonRenderLight 0x%I64X", pNewLight->m_CommonRenderLight);
//       Sleep(1000);
//       pTestLight = pNewLight;
//     }

    m_InputSystem->Update();
    m_CameraManager->Update(dt.count());
    m_UI->Update(dt.count());

    // Check if config has been affected, if so, save it
    m_dtConfigCheck += dt.count();
    if (m_dtConfigCheck > 10.f)
    {
      m_dtConfigCheck = 0;
      if (m_ConfigChanged)
      {
        m_ConfigChanged = false;
        SaveConfig();
      }
    }

    Sleep(10);
  }
}

void Main::LoadConfig()
{
  // Read config.ini using inih by Ben Hoyt
  // https://github.com/benhoyt/inih

  m_Config = std::make_unique<INIReader>(g_configFile);
  int parseResult = m_Config->ParseError();

  // If there's problems reading the file, notify the user.
  // Code-wise it should be safe to just continue,
  // since you can still request variables from INIReader.
  // They'll just return the specified default value.
  if (parseResult != 0)
    util::log::Warning("Config file could not be loaded, using default settings");

  m_CameraManager->ReadConfig(m_Config.get());
  m_InputSystem->ReadConfig(m_Config.get());
}

void Main::SaveConfig()
{
  std::fstream file;
  file.open(g_configFile, std::ios_base::out | std::ios_base::trunc);

  if (!file.is_open())
  {
    util::log::Error("Could not save config, failed to open file for writing. GetLastError 0x%X", GetLastError());
    return;
  }

  file << m_CameraManager->GetConfig();
  file << m_InputSystem->GetConfig();

  file.close();
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam); 
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
    return TRUE;

  switch (uMsg)
  {
    case WM_ACTIVATE:
    {
      g_hasFocus = (wParam != WA_INACTIVE);
      break;
    }
    case WM_INPUT:
    {
      if (!g_mainHandle->GetUI()->IsEnabled())
        g_mainHandle->GetInputSystem()->HandleRawInput(lParam);
      else// if (g_mainHandle->GetUI()->HasKeyboardFocus())
        return TRUE;
      break;
    }
    case WM_KEYDOWN:
    {
      if (g_mainHandle->GetInputSystem()->HandleKeyMsg(wParam, lParam) ||
        g_mainHandle->GetUI()->HasKeyboardFocus())
        return TRUE;
      break;
    }
    case WM_SIZE:
    {
      // Resize event
      g_mainHandle->GetRenderer()->OnResize();
      break;
    }
    case WM_DESTROY:
    {
      g_shutdown = true;
      break;
    }
  }

  return CallWindowProc(g_origWndProc, hwnd, uMsg, wParam, lParam);
}


// void Main::RenderTestLight()
// {
//   if (!pTestLight) return;
// 
//   pTestLight->m_CommonRenderLight->ShadowmapUpdate();
//   Foundation::LightRenderCallback::Singleton()->RenderLight(pTestLight);
//   //pTestLight->m_CommonRenderLight->ShadowmapUpdate();
// }

const unsigned char Foundation::CommonLightResource::DefaultVoxelData[0xFF] = 
{ 0xFF, 0xFE, 0xFC, 0xFB, 0xFA, 0xF8, 0xF7, 0xF6, 0xF4, 0xF3, 0xF2, 0xF0, 0xEF, 0xED, 0xEC, 0xEB,
0xE9, 0xE8, 0xE7, 0xE5, 0xE4, 0xE2, 0xE1, 0xE0, 0xDE, 0xDD, 0xDB, 0xDA, 0xD8, 0xD7, 0xD6, 0xD4,
0xD3, 0xD1, 0xD0, 0xCF, 0xCD, 0xCC, 0xCA, 0xC9, 0xC7, 0xC6, 0xC5, 0xC3, 0xC2, 0xC0, 0xBF, 0xBD,
0xBC, 0xBA, 0xB9, 0xB8, 0xB6, 0xB5, 0xB3, 0xB2, 0xB0, 0xAF, 0xAE, 0xAC, 0xAB, 0xA9, 0xA8, 0xA6,
0xA5, 0xA4, 0xA2, 0xA1, 0x9F, 0x9E, 0x9C, 0x9B, 0x9A, 0x98, 0x97, 0x95, 0x94, 0x93, 0x91, 0x90,
0x8E, 0x8D, 0x8C, 0x8A, 0x89, 0x88, 0x86, 0x85, 0x83, 0x82, 0x81, 0x7F, 0x7E, 0x7D, 0x7B, 0x7A,
0x79, 0x77, 0x76, 0x75, 0x73, 0x72, 0x71, 0x6F, 0x6E, 0x6D, 0x6B, 0x6A, 0x69, 0x68, 0x66, 0x65,
0x64, 0x62, 0x61, 0x60, 0x5F, 0x5E, 0x5C, 0x5B, 0x5A, 0x59, 0x57, 0x56, 0x55, 0x54, 0x53, 0x52,
0x50, 0x4F, 0x4E, 0x4D, 0x4C, 0x4B, 0x4A, 0x48, 0x47, 0x46, 0x45, 0x44, 0x43, 0x42, 0x41, 0x40,
0x3F, 0x3E, 0x3D, 0x3C, 0x3B, 0x3A, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x31,
0x30, 0x2F, 0x2E, 0x2D, 0x2D, 0x2C, 0x2B, 0x2A, 0x2A, 0x29, 0x28, 0x28, 0x27, 0x26, 0x26, 0x25,
0x24, 0x24, 0x23, 0x23, 0x22, 0x21, 0x21, 0x20, 0x20, 0x1F, 0x1F, 0x1E, 0x1E, 0x1D, 0x1D, 0x1C,
0x1C, 0x1B, 0x1B, 0x1A, 0x1A, 0x19, 0x19, 0x18, 0x18, 0x17, 0x17, 0x17, 0x16, 0x16, 0x15, 0x15,
0x14, 0x14, 0x14, 0x13, 0x13, 0x12, 0x12, 0x12, 0x11, 0x11, 0x10, 0x10, 0x10, 0x0F, 0x0F, 0x0E,
0x0E, 0x0E, 0x0D, 0x0D, 0x0C, 0x0C, 0x0C, 0x0B, 0x0B, 0x0A, 0x0A, 0x0A, 0x09, 0x09, 0x08, 0x08,
0x07, 0x07, 0x06, 0x06, 0x06, 0x05, 0x05, 0x04, 0x04, 0x03, 0x03, 0x02, 0x02, 0x01, 0x01 };