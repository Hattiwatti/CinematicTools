#pragma once

#include <DirectXMath.h>
#include <string>
#include <Windows.h>

namespace util
{
  namespace hooks
  {
    void Init();

    void DisableHooks();
    void EnableHooks();

    void RemoveHooks();
    void Uninitialize();
  };

  namespace log
  {
    void Init();

    void Write(const char* format, ...);
    void Warning(const char* format, ...);
    void Error(const char* format, ...);
    void Ok(const char* format, ...);
  };

  bool GetResource(int, void*&, DWORD&);
}