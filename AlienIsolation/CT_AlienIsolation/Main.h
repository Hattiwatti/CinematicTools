#pragma once
#include <Windows.h>
#include <memory>
#include <boost/chrono.hpp>

#include "Camera/CameraManager.h"
#include "Input/InputManager.h"
#include "UIManager.h"

using namespace boost::chrono;

class Main
{
public:
  Main();
  ~Main();
  void Init(HINSTANCE);

  bool IsExiting() { return m_exit; }

  HINSTANCE GetDllHandle() { return m_dllHandle; }
  high_resolution_clock* GetClock() { return &m_Clock; }

  CameraManager* GetCameraManager() { return m_pCameraManager.get(); }
  InputManager* GetInputManager() { return m_pInputManager.get(); }
  UIManager* GetUIManager() { return m_pUIManager.get(); }

private:
  HINSTANCE m_dllHandle;
  bool m_exit;

  std::unique_ptr<CameraManager> m_pCameraManager;
  std::unique_ptr<InputManager> m_pInputManager;
  std::unique_ptr<UIManager> m_pUIManager;

  void Update();

  high_resolution_clock m_Clock;
  high_resolution_clock::time_point m_dtUpdate;

public:
  Main(Main const&) = delete;
  void operator=(Main const&) = delete;
};

extern Main* g_mainHandle;