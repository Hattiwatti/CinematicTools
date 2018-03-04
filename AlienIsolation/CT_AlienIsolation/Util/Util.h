#pragma once

#include <string>
#include <Windows.h>

namespace util
{
  namespace hooks
  {
    bool Init();

    void DisableHooks();
    void EnableHooks();

    void Release();
  }

  namespace log
  {
    void Init();

    void Write(const char* format, ...);
    void Warning(const char* format, ...);
    void Error(const char* format, ...);
    void Ok(const char* format, ...);
  }

  bool GetResource(int, void*&, DWORD&);
  BOOL WriteMemory(DWORD_PTR, const void*, DWORD);
}