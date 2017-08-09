#pragma once
#include <Windows.h>
#include <memory>

#include "Camera/CameraManager.h"
#include "UIManager.h"

class Main
{
public:
  Main();
  ~Main();
  void Init(HINSTANCE);

  bool IsExiting() { return m_exit; }

  HINSTANCE GetDllHandle() { return m_dllHandle; }

  CameraManager* GetCameraManager() { return m_pCameraManager.get(); }
  UIManager* GetUIManager() { return m_pUIManager.get(); }

private:
  HINSTANCE m_dllHandle;
  bool m_exit;

  std::unique_ptr<CameraManager> m_pCameraManager;
  std::unique_ptr<UIManager> m_pUIManager;


  void Update();
public:
  Main(Main const&) = delete;
  void operator=(Main const&) = delete;
};

extern Main* g_mainHandle;