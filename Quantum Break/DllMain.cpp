#include "Main.h"

DWORD WINAPI RunCT(LPVOID lpArg)
{
  g_mainHandle = new Main();
  if (g_mainHandle->Initialize())
    g_mainHandle->Run();

  delete g_mainHandle;

  FreeLibraryAndExitThread(g_dllHandle, 0);
  return 0;
}

DWORD WINAPI DllMain(_In_ HINSTANCE hInstance, _In_ DWORD fdwReason, _In_ LPVOID lpvReserved)
{
  if (fdwReason == DLL_PROCESS_ATTACH)
  {
    g_dllHandle = hInstance;
    CreateThread(NULL, NULL, RunCT, NULL, NULL, NULL);
  }

  return 1;
}