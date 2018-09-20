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

Foundation::SceneLight* pTestLight = nullptr;
Foundation::SceneEntityData* pEntityData = nullptr;
Foundation::CommonLightResource* pResource = nullptr;

void Main::Run()
{
  boost::chrono::high_resolution_clock::time_point lastUpdate = boost::chrono::high_resolution_clock::now();
  while (!g_shutdown)
  {
    boost::chrono::duration<double> dt = boost::chrono::high_resolution_clock::now() - lastUpdate;
    lastUpdate = boost::chrono::high_resolution_clock::now();

    if (GetKeyState(VK_F8) & 0x8000)
    {
      if (!pResource)
        pResource = Foundation::CommonLightResource::Create();

      if (!pEntityData)
        pEntityData = new Foundation::SceneEntityData();

      util::log::Write("CommonLightResource 0x%I64X", pResource);
      util::log::Write("SceneEntityData 0x%I64X", pEntityData);
      Foundation::SceneLight* pNewLight = Foundation::Scene::Singleton()->CreateLight(pResource);

      util::log::Write("SceneLight 0x%I64X", pNewLight);
      pNewLight->SetEntityData(pEntityData);

      pNewLight->m_CommonRenderLight->m_SceneLight = pNewLight;
      pNewLight->m_CommonRenderLight->m_LightResource = pResource;
      pNewLight->m_CommonRenderLight->m_Color = DirectX::XMFLOAT4(1, 1, 1, 1);
      pNewLight->m_CommonRenderLight->m_AttenuationDistance = 500.f;
      pNewLight->m_CommonRenderLight->m_Flag2 = 0xC4;

      Foundation::GameRender* pGameRender = Foundation::GameRender::Singleton();
      pNewLight->m_CommonRenderLight->m_Transform = DirectX::XMMatrixIdentity();
      pNewLight->m_CommonRenderLight->m_Transform.r[3] = pGameRender->m_CameraTransform.r[3];

      util::log::Write("CommonRenderLight 0x%I64X", pNewLight->m_CommonRenderLight);
      Sleep(1000);
      pTestLight = pNewLight;
    }

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
      else if (g_mainHandle->GetUI()->HasKeyboardFocus())
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


void Main::RenderTestLight()
{
  if (!pTestLight) return;

  Foundation::LightRenderCallback::Singleton()->RenderLight(pTestLight);
}