#pragma once
#include <Windows.h>
#include "Rendering/Dx11Renderer.h"
#include "UI.h"

class Main
{
public:
  Main();
  ~Main();

  bool Initialize();
  void Release();
  void Run();

  Dx11Renderer* GetRenderer() { return m_pRenderer.get(); }
  UI* GetUI() { return m_pUI.get(); }

private:

private:
  std::unique_ptr<Dx11Renderer> m_pRenderer;
  std::unique_ptr<UI> m_pUI;

public:
  Main(Main const&) = delete;
  void operator=(Main const&) = delete;
};

extern Main* g_mainHandle;
extern HINSTANCE g_dllHandle;
extern bool g_shutdown;