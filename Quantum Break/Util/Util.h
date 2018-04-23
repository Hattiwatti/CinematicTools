#pragma once

#include <DirectXMath.h>
#include <map>
#include <shtypes.h>
#include <stdio.h>
#include <string>
#include <vector>
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
  PBYTE WINAPI HookVTableFunction(PDWORD64* ppVTable, PBYTE pHook, SIZE_T iIndex);
}