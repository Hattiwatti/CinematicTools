#include <Windows.h>
#include "Main.h"

bool g_shutdown = false;
Main* g_mainHandle = nullptr;
HINSTANCE g_dllHandle = NULL;
HWND g_gameHwnd = NULL;

DWORD WINAPI ShutdownThread(LPVOID lpArg)
{
  while (!(GetAsyncKeyState(VK_F6) & 0x8000))
    Sleep(100);

  g_shutdown = true;
  while (g_shutdown)
    Sleep(1000);

  FreeLibraryAndExitThread(g_dllHandle, 0);
}

DWORD WINAPI InitializeThread(LPVOID lpArg)
{
  HINSTANCE* pInstance = static_cast<HINSTANCE*>(lpArg);
  g_dllHandle = *pInstance;

  g_mainHandle = new Main();
  if (g_mainHandle->Initialize())
    g_mainHandle->Run();

  g_mainHandle->Release();

  delete g_mainHandle;
  delete pInstance;
  g_shutdown = false;

  return 0;
}

DWORD WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
    CreateThread(NULL, NULL, InitializeThread, new HINSTANCE(hInstance), NULL, NULL);
    CreateThread(NULL, NULL, ShutdownThread, new HINSTANCE(hInstance), NULL, NULL);
	}

	return 1;
}