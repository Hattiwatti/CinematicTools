#include "Main.h"

Main* g_mainHandle = nullptr;
HINSTANCE g_dllHandle = NULL;
bool g_shutdown = false;

DWORD WINAPI Initialize(LPVOID arg)
{
  HINSTANCE* pHandle = reinterpret_cast<HINSTANCE*>(arg);
  g_dllHandle = *pHandle;

  g_mainHandle = new Main();
  if (g_mainHandle->Initialize())
    g_mainHandle->Run();

  g_mainHandle->Release();
  
  delete g_mainHandle;
  delete pHandle;
  g_mainHandle = nullptr;
  g_shutdown = false;

  return 0;
}

DWORD WINAPI ShutdownThread(LPVOID arg)
{
  while (!(GetAsyncKeyState(VK_F5) & 0x8000))
    Sleep(100);

  g_shutdown = true;
  while (g_shutdown)
    Sleep(100);

  FreeLibraryAndExitThread(g_dllHandle, 0);
}

DWORD WINAPI DllMain(HINSTANCE handle, DWORD dwReason, LPVOID lpReserved)
{
  if (dwReason == DLL_PROCESS_ATTACH)
  {
    CreateThread(NULL, NULL, Initialize, new HINSTANCE(handle), NULL, NULL);
    CreateThread(NULL, NULL, ShutdownThread, NULL, NULL, NULL);
  }

  return 1;
}