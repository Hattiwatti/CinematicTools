#include "../AlienIsolation.h"
#include "../Main.h"
#include "Util.h"
#include <MinHook.h>

#pragma comment(lib, "libMinHook.x86.lib")

using namespace util;

typedef HRESULT(WINAPI * tDXGIPresent)(IDXGISwapChain*, UINT, UINT);

tDXGIPresent oDXGIPresent = nullptr;
HRESULT WINAPI hDXGIPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
  return oDXGIPresent(pSwapChain, SyncInterval, Flags);
}

PBYTE WINAPI HookVTableFunction(PDWORD* ppVTable, PBYTE pHook, SIZE_T iIndex)
{
  DWORD dwOld = 0;
  VirtualProtect((void*)((*ppVTable) + iIndex), sizeof(PDWORD), PAGE_EXECUTE_READWRITE, &dwOld);

  PBYTE pOrig = ((PBYTE)(*ppVTable)[iIndex]);
  (*ppVTable)[iIndex] = (DWORD)pHook;

  VirtualProtect((void*)((*ppVTable) + iIndex), sizeof(PDWORD), dwOld, &dwOld);

  return pOrig;
}

bool hooks::Init()
{
  log::Write("Initializing hooks");

  MH_STATUS status = MH_Initialize();
  if (status != MH_OK)
  {
    log::Error("Could not initialize MinHook. MH_STATUS 0x%X", status);
    return false;
  }

  oDXGIPresent = (tDXGIPresent)HookVTableFunction((PDWORD*)AI::D3D::Singleton()->m_pSwapChain, (PBYTE)hDXGIPresent, 8);

  log::Ok("Hooks initialized");
}

void hooks::EnableHooks()
{
  MH_STATUS status = MH_EnableHook(MH_ALL_HOOKS);
  if (status != MH_OK)
    log::Error("Could not enable all hooks. MH_STATUS 0x%X", status);

  HookVTableFunction((PDWORD*)AI::D3D::Singleton()->m_pSwapChain, (PBYTE)hDXGIPresent, 8);
}

void hooks::DisableHooks()
{
  MH_STATUS status = MH_DisableHook(MH_ALL_HOOKS);
  if (status != MH_OK)
    log::Error("Could not disable all hooks. MH_STATUS 0x%X", status);

  HookVTableFunction((PDWORD*)AI::D3D::Singleton()->m_pSwapChain, (PBYTE)oDXGIPresent, 8);
}

void hooks::Release()
{
  DisableHooks();

  MH_STATUS status = MH_RemoveHook(MH_ALL_HOOKS);
  if (status != MH_OK)
    log::Error("Could not remove all hooks. MH_STATUS 0x%X", status);

  status = MH_Uninitialize();
  if (status != MH_OK)
    log::Error("Could not uninitialize MinHook. MH_STATUS 0x%X", status);
}