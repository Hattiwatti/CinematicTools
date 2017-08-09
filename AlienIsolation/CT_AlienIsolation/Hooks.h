#pragma once
#include <Windows.h>

class Hooks
{
public:
  static void Init();

  static void DisableHooks();
  static void RemoveHooks();
  static void UnInitialize();

private:
  static void CreateHook(const char*, int, PVOID, LPVOID*);
  static PBYTE WINAPI HookVTableFunction(PDWORD*, PBYTE, SIZE_T);
};

extern Hooks* g_hooksHandle;