#pragma once
#include "Input\InputManager.h"
#include "Modules\CameraManager.h"
#include "Modules\VisualManager.h"
#include "UIManager.h"
#include <memory>
#include <Windows.h>

class Main
{
public:
  Main();
  ~Main();

  void Initialize();
  void Release();

  CameraManager* GetCameraManager() { return m_pCameraManager.get(); }
  VisualManager* GetVisualManager() { return m_pVisualManager.get(); }

  InputManager* GetInputManager() { return m_pInputManager.get(); }
  UIManager* GetUIManager() { return m_pUIManager.get(); }

private:
  std::unique_ptr<CameraManager> m_pCameraManager;
  std::unique_ptr<VisualManager> m_pVisualManager;
  
  std::unique_ptr<InputManager> m_pInputManager;
  std::unique_ptr<UIManager> m_pUIManager;

  bool m_shutdown;

public:
  Main(Main const&) = delete;
  void operator=(Main const&) = delete;
};

extern Main* g_mainHandle;
extern HINSTANCE g_dllHandle;
extern bool g_gameUIDisabled;
extern bool g_shutdown;