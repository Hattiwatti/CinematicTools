#include "Main.h"

Main* g_mainHandle = nullptr;

DWORD WINAPI Initialize(LPVOID arg)
{
  HINSTANCE* pDllInstance = static_cast<HINSTANCE*>(arg);
 
  g_mainHandle = new Main();
  g_mainHandle->Init(*pDllInstance);

  delete pDllInstance;
  delete g_mainHandle;
  g_mainHandle = 0;

  return 0;
}

DWORD WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
  if (dwReason == DLL_PROCESS_ATTACH)
    CreateThread(0, 0, &Initialize, new HINSTANCE(hInstance), 0, 0);

  return 1;
}