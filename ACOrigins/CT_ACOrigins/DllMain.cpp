#include <Windows.h>

DWORD WINAPI Initialize(LPVOID lpArg)
{


  return 0;
}

DWORD WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
  if (dwReason == DLL_PROCESS_ATTACH)
    CreateThread(NULL, NULL, Initialize, new HINSTANCE(hInstance), NULL, NULL);

  return 1;
}