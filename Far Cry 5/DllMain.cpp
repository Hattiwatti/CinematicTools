#include "Main.h"

Main* g_mainHandle = nullptr;
HINSTANCE g_dllHandle = NULL;
bool g_shutdown = false;

DWORD WINAPI Initialize(LPVOID lpArg)
{
  HINSTANCE* pDllInstance = static_cast<HINSTANCE*>(lpArg);
  g_dllHandle = *pDllInstance;

  g_mainHandle = new Main();
  if (g_mainHandle->Initialize())
    g_mainHandle->Run();

  g_mainHandle->Release();
  FreeLibraryAndExitThread(g_dllHandle, 0);

  delete pDllInstance;
  delete g_mainHandle;

  return 0;
}

DWORD WINAPI DllMain(_In_ HINSTANCE _DllHandle, _In_ unsigned long _Reason, _In_opt_ void* _Reserved)
{
  if (_Reason == DLL_PROCESS_ATTACH)
    CreateThread(NULL, NULL, Initialize, new HINSTANCE(_DllHandle), NULL, NULL);

  return 1;
}