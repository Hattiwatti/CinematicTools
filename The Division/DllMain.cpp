#include <stdio.h>
#include <Windows.h>

#include "Main.h"

Main* g_mainHandle = nullptr;
HINSTANCE g_dllHandle = NULL;
bool g_shutdown = false;

DWORD WINAPI ShutdownThread(LPVOID arg)
{
  while (true)
  {
    if (GetAsyncKeyState(VK_F10) & 0x8000)
      break;

    Sleep(100);
  }

  g_shutdown = true;
  Sleep(2000);

  FreeLibraryAndExitThread(g_dllHandle, 0);
  return 1;
}

DWORD WINAPI Initialize(LPVOID arg)
{
  HINSTANCE* pDllHandle = static_cast<HINSTANCE*>(arg);
  g_dllHandle = *pDllHandle;

  g_mainHandle = new Main();
  g_mainHandle->Initialize();

  g_mainHandle->Release();
  delete g_mainHandle;

  return 1;
}

DWORD WINAPI DllMain(_In_ HINSTANCE _DllHandle, _In_ unsigned long _Reason, _In_opt_ void* _Reserved)
{
  if (_Reason == DLL_PROCESS_ATTACH)
  {
    CreateThread(0, 0, &Initialize, new HINSTANCE(_DllHandle), 0, 0);
    //CreateThread(0, 0, &ShutdownThread, 0, 0, 0);
  }

  return 1;
}