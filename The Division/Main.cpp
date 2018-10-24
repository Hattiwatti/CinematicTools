#include "Main.h"
#include "Util/Util.h"
#include "Modules/Snowdrop.h"
#include <boost/chrono.hpp>

bool g_gameUIDisabled = false;
__int64 g_pBase = 0;

Main::Main()
{

}

Main::~Main()
{

}

void Main::Initialize()
{
  util::log::Init();
  util::log::Write("Cinematic Tools for Tom Clancy's The Division");

  Sleep(1000);

  g_pBase = reinterpret_cast<__int64>(GetModuleHandleA("TheDivision.exe"));
  if (g_pBase == 0)
    g_pBase = reinterpret_cast<__int64>(GetModuleHandleA("thedivision.exe"));

  if (g_pBase == 0)
  {
    util::log::Error("Failed to locate TheDivision.exe base address!!!");
    Sleep(5000);
  }

  m_pCameraManager = std::make_unique<CameraManager>();
  m_pVisualManager = std::make_unique<VisualManager>();

  m_pInputManager = std::make_unique<InputManager>();
  m_pUIManager = std::make_unique<UIManager>();

  m_pInputManager->Initialize();
  util::hooks::Init();

  boost::chrono::high_resolution_clock::time_point lastUpdate = boost::chrono::high_resolution_clock::now();

  while (!g_shutdown)
  {
    if (m_pInputManager->IsKeyDown(InputManager::Action::Camera_Toggle))
    {
      m_pCameraManager->ToggleCamera();

      while (m_pInputManager->IsKeyDown(InputManager::Action::Camera_Toggle))
        Sleep(1);
    }

    if (m_pInputManager->IsKeyDown(InputManager::Action::Camera_FreezeTime))
    {
      TD::TimeModule* pTimeModule = TD::TimeModule::Singleton();
      if (pTimeModule->m_TimeScale == 1.f)
        pTimeModule->m_TimeScale = 0.0001f;
      else
        pTimeModule->m_TimeScale = 1.0f;

      while (m_pInputManager->IsKeyDown(InputManager::Action::Camera_FreezeTime))
        Sleep(1);
    }

    if (m_pInputManager->IsKeyDown(InputManager::Action::Camera_ToggleUI))
    {
      g_gameUIDisabled = !g_gameUIDisabled;

      while (m_pInputManager->IsKeyDown(InputManager::Action::Camera_ToggleUI))
        Sleep(1);
    }

    if (m_pInputManager->IsKeyDown(InputManager::Action::UI_Toggle))
    {
      m_pUIManager->ToggleUI();

      while (m_pInputManager->IsKeyDown(InputManager::Action::UI_Toggle))
        Sleep(1);
    }

    boost::chrono::duration<double> dt = boost::chrono::high_resolution_clock::now() - lastUpdate;
    lastUpdate = boost::chrono::high_resolution_clock::now();

    m_pCameraManager->Update(dt.count());
    Sleep(1);
  }
}

void Main::Release()
{
  util::hooks::DisableHooks();
 
  m_pUIManager->Release();
  m_pInputManager->Release();
}