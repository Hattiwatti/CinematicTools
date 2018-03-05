#pragma once
#include <memory.h>
#include <Windows.h>

#include "Rendering/Dx11Renderer.h"
#include "UI.h"

class Main
{
public:
  Main() {};
  ~Main() {};

  bool Initialize();
  void Run();
  void Release();

  UI* GetUI() { return m_pUI.get(); }

private:
  std::unique_ptr<Dx11Renderer> m_pRenderer;
  std::unique_ptr<UI> m_pUI;

public:
  Main(Main const&) = delete;
  void operator=(Main const&) = delete;
};

extern bool g_shutdown;

extern Main* g_mainHandle;
extern HINSTANCE g_dllHandle;
extern HWND g_gameHwnd;