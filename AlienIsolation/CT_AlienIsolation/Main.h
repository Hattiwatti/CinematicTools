#pragma once
#include <Windows.h>

class Main
{
public:
  Main();
  ~Main();

  void Init(HINSTANCE);

  HINSTANCE GetDllHandle() { return m_dllHandle; }

private:
  HINSTANCE m_dllHandle;

public:
  Main(Main const&) = delete;
  void operator=(Main const&) = delete;
};

extern Main* g_mainHandle;