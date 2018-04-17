#pragma once
#include <boost/chrono.hpp>
#include <memory>
#include <Windows.h>

#include "Config.h"
#include "Modules/CameraManager.h"
#include "Modules/EnvironmentManager.h"
#include "Modules/InputManager.h"
#include "UIManager.h"

class Main
{
public:
  Main();
  ~Main();

  bool Initialize();
  void Release();
  void Run();

  CameraManager* GetCameraManager() { return m_pCameraManager.get(); }
  Config* GetConfig() { return m_pConfig.get(); }
  EnvironmentManager* GetEnvironmentManager() { return m_pEnvironmentManager.get(); }
  InputManager* GetInputManager() { return m_pInputManager.get(); }
  UI* GetUI() { return m_pUI.get(); }

private:
  void Update();

private:
  boost::chrono::high_resolution_clock::time_point m_dtUpdate;

  std::unique_ptr<CameraManager> m_pCameraManager;
  std::unique_ptr<Config> m_pConfig;
  std::unique_ptr<EnvironmentManager> m_pEnvironmentManager;
  std::unique_ptr<InputManager> m_pInputManager;
  std::unique_ptr<UI> m_pUI;

public:
  Main(Main const&) = delete;
  void operator=(Main const&) = delete;
};

extern Main* g_mainHandle;
extern HINSTANCE g_dllHandle;
extern bool g_shutdown;