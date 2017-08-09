#include "Hooks.h"
#include "Main.h"
#include "AlienIsolation.h"
#include "Util/Log.h"

#include <MinHook.h>
#pragma comment(lib, "libMinHook.x86.lib")

typedef HRESULT(WINAPI * tD3D11Present)(IDXGISwapChain*, UINT, UINT);
typedef int(__thiscall* tCameraUpdate)(int, int, int);
typedef int(__fastcall* tCameraUpdate2)(int, int, int);

tD3D11Present oD3D11Present = nullptr;
tCameraUpdate oCameraUpdate = nullptr;
tCameraUpdate2 oCameraUpdate2 = nullptr;

HRESULT WINAPI hD3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
  g_mainHandle->GetUIManager()->Draw();
  return oD3D11Present(pSwapChain, SyncInterval, Flags);
}

int __fastcall hCameraUpdate(int This, int _EDX, int a1, int a2)
{
  return oCameraUpdate(This, a1, a2);
}

int __fastcall hCameraUpdate2(int a1, int a2, int a3)
{
  int result = oCameraUpdate2(a1, a2, a3);
  g_mainHandle->GetCameraManager()->CameraHook(a3);
  return result;
}

//
// Creates a normal function hook with MinHook, 
// which places a jmp instruction at the start of the function.
//
void Hooks::CreateHook(const char* name, int target, PVOID hook, LPVOID* original)
{
  MH_STATUS result = MH_CreateHook((LPVOID)target, hook, original);
  if (result != MH_OK)
  {
    Log::Error("Could not create %s hook. MH_STATUS 0x%X", name, result);
    return;
  }
  result = MH_EnableHook((LPVOID)target);
  if (result != MH_OK)
  {
    Log::Error("Could not enable %s hook. MH_STATUS 0x%X", name, result);
    return;
  }
}

//
// Hooks a function by changing the address at given index
// in the virtual function table
//
PBYTE WINAPI Hooks::HookVTableFunction(PDWORD* ppVTable, PBYTE pHook, SIZE_T iIndex)
{
  DWORD dwOld = 0;
  VirtualProtect((void*)((*ppVTable) + iIndex), sizeof(PDWORD), PAGE_EXECUTE_READWRITE, &dwOld);

  PBYTE pOrig = ((PBYTE)(*ppVTable)[iIndex]);
  (*ppVTable)[iIndex] = (DWORD)pHook;

  VirtualProtect((void*)((*ppVTable) + iIndex), sizeof(PDWORD), dwOld, &dwOld);

  return pOrig;
}

void Hooks::Init()
{
  MH_STATUS status = MH_Initialize();
  if (status != MH_OK)
  {
    Log::Error("Could not initialize MinHook. MH_STATUS 0x%X", status);
    return;
  }

  CreateHook("Camera Update", ((int)GetModuleHandleA("AI.exe") + 0x2CCF0), hCameraUpdate, (LPVOID*)&oCameraUpdate);
  CreateHook("Camera Update #2", ((int)GetModuleHandleA("AI.exe") + 0x2ADA0), hCameraUpdate2, (LPVOID*)&oCameraUpdate2);
  oD3D11Present = (tD3D11Present)HookVTableFunction((PDWORD*)AI::Rendering::GetSwapChain(), (PBYTE)hD3D11Present, 8);
}

void Hooks::DisableHooks()
{
  MH_STATUS status = MH_DisableHook(MH_ALL_HOOKS);
  if (status != MH_OK)
    Log::Error("Could not disable all hooks. MH_STATUS 0x%X", status);
}

void Hooks::RemoveHooks()
{
  MH_STATUS status = MH_RemoveHook(MH_ALL_HOOKS);
  if (status != MH_OK)
    Log::Error("Could not disable all hooks. MH_STATUS 0x%X", status);
}

void Hooks::UnInitialize()
{
  RemoveHooks();
  MH_STATUS status = MH_Uninitialize();
  if (status != MH_OK)
    Log::Error("Could not uninitialize MinHook. MH_STATUS 0x%X", status);
}
