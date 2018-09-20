#pragma once
#include "Camera/CameraManager.h"
#include "Input/InputSystem.h"
#include "Rendering/CTRenderer.h"
#include "UI.h"

#include "inih/cpp/INIReader.h"
#include <memory.h>

class Main
{
public:
  Main();
  ~Main();

  bool Initialize();
  void Run();

  CameraManager*  GetCameraManager()  { return m_CameraManager.get(); }
  InputSystem*    GetInputSystem()    { return m_InputSystem.get(); }
  CTRenderer*     GetRenderer()       { return m_Renderer.get(); }
  UI*             GetUI()             { return m_UI.get(); }

  void OnConfigChanged() { m_ConfigChanged = true; }

private:
  void LoadConfig();
  void SaveConfig();

private:

  std::unique_ptr<INIReader>      m_Config;
  std::unique_ptr<CameraManager>  m_CameraManager;
  std::unique_ptr<InputSystem>    m_InputSystem;
  std::unique_ptr<CTRenderer>     m_Renderer;
  std::unique_ptr<UI>             m_UI;

  bool m_Initialized;
  bool m_ConfigChanged;
  float m_dtConfigCheck;

public:
  Main(Main const&) = delete;
  void operator=(Main const&) = delete;
};