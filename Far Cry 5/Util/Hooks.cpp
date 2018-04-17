#include "Util.h"
#include "../Main.h"
#include "../Dunya.h"

#include <MinHook.h>
#include <map>
#include <vector>

#pragma comment(lib, "libMinHook.x64.lib")

using namespace util;

////////////////////////////////
////  FUNCTION DEFINITIONS  ////
////////////////////////////////

typedef HCURSOR(WINAPI* tSetCursor)(HCURSOR);
typedef BOOL(WINAPI* tSetCursorPos)(int x, int y);
typedef HRESULT(WINAPI * tD3D11Present)(IDXGISwapChain*, UINT, UINT);
typedef __int64(__fastcall* tOnResize)(__int64, __int64);
typedef __int64(__fastcall* tComponentIterator)(__int64, __int64, __int64);
typedef __int64(__fastcall* tCameraUpdate)(__int64, __int64, __int64);
typedef __int64(__fastcall* tCameraAngles)(__int64, __int64, __int64);

typedef __int64(__fastcall* tGamepadUpdate)(__int64, int, __int64);
typedef __int64(__fastcall* tInputUpdate)(__int64, __int64, char);
typedef __int64(__fastcall* tDrawFireUI)(__int64, __int64, __int64, __int64);


////////////////////////////////
////  RENDER-RELATED HOOKS  ////
////////////////////////////////

tD3D11Present oD3D11Present = nullptr;
tOnResize oOnResize = nullptr;
tDrawFireUI oDrawFireUI = nullptr;

HRESULT WINAPI hD3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
  g_mainHandle->GetUI()->Draw();

  return oD3D11Present(pSwapChain, SyncInterval, Flags);
}

__int64 __fastcall hOnResize(__int64 a1, __int64 a2)
{
  g_mainHandle->GetUI()->ResizeBuffers(true);
  return oOnResize(a1, a2);
}

__int64 __fastcall hDrawFireUI(__int64 a1, __int64 a2, __int64 a3, __int64 a4)
{
  __int64 pSettings = *(__int64*)(a1 + 0x1D8);
  BYTE* pDisableFireUI = (BYTE*)(pSettings + 0x16C);
  *pDisableFireUI = g_mainHandle->GetCameraManager()->IsGameUIDisabled();

  return oDrawFireUI(a1, a2, a3, a4);
}

////////////////////////////////
////   GAME FUNCTIONALITY   ////
////////////////////////////////

tComponentIterator oComponentIterator = nullptr;
tCameraUpdate oCameraUpdate = nullptr;
tCameraAngles oCameraAngles = nullptr;
tGamepadUpdate oGamepadUpdate = nullptr;
tInputUpdate oInputUpdate = nullptr;

__int64 __fastcall hComponentIterator(__int64 a1, __int64 a2, __int64 a3)
{
  FC::ComponentCollection<__int64>* pCollection = (FC::ComponentCollection<__int64>*)a3;
  g_mainHandle->GetCameraManager()->ComponentHook(pCollection);

  return oComponentIterator(a1, a2, a3);
}

__int64 __fastcall hCameraUpdate(__int64 a1, __int64 a2, __int64 a3)
{
  __int64 result = oCameraUpdate(a1, a2, a3);

  g_mainHandle->GetCameraManager()->CameraHook((FC::CMarketingCamera*)a1);
  return result;
}

__int64 __fastcall hCameraAngles(__int64 a1, __int64 a2, __int64 a3)
{
  __int64 result = oCameraAngles(a1, a2, a3);
  g_mainHandle->GetCameraManager()->AngleHook(a1);
  return result;
}

__int64 __fastcall hGamepadUpdate(__int64 a1, int a2, __int64 a3)
{
  if (g_mainHandle->GetCameraManager()->IsCameraEnabled() &&
    g_mainHandle->GetCameraManager()->IsGamepadDisabled())
    return 0;

  return oGamepadUpdate(a1, a2, a3);
}

__int64 __fastcall hInputUpdate(__int64 a1, __int64 a2, char a3)
{
  if (g_mainHandle->GetCameraManager()->IsCameraEnabled() ||
      g_mainHandle->GetUI()->IsEnabled())
    return 0;

  return oInputUpdate(a1, a2, a3);
}

tSetCursor oSetCursor = nullptr;
tSetCursorPos oSetCursorPos = nullptr;

HCURSOR WINAPI hSetCursor(HCURSOR cursor)
{
  if (cursor == NULL)
  {
    if (g_mainHandle->GetUI()->IsEnabled())
      return oSetCursor(g_mainHandle->GetUI()->GetCursor());
  }

  return oSetCursor(cursor);
}

BOOL WINAPI hSetCursorPos(int x, int y)
{
  if (g_mainHandle->GetUI()->IsEnabled())
    return TRUE;

  return oSetCursorPos(x, y);
}

std::map<std::string, __int64> createdHooks;

//
// Creates a normal function hook with MinHook, 
// which places a jmp instruction at the start of the function.
//
template <typename T>
void CreateHook(const char* name, __int64 target, PVOID hook, T original)
{
  LPVOID* pOriginal = reinterpret_cast<LPVOID*>(original);
  MH_STATUS result = MH_CreateHook((LPVOID)target, hook, pOriginal);
  if (result != MH_OK)
  {
    log::Error("Could not create %s hook. MH_STATUS 0x%X error code 0x%X", name, result, GetLastError());
    log::Error("Retrying in 5 seconds..."); Sleep(5000);
    result = MH_CreateHook((LPVOID)target, hook, pOriginal);
    if (result != MH_OK)
    {
      log::Error("Still failed to create the hook");
      return;
    }
    else
      log::Ok("Hook created successfully, continuing...");
  }
  result = MH_EnableHook((LPVOID)target);
  if (result != MH_OK)
  {
    log::Error("Could not enable %s hook. MH_STATUS 0x%X error code 0x%X", name, result, GetLastError());
    log::Error("Retrying in 5 seconds...");  Sleep(5000);
    result = MH_EnableHook((LPVOID)target);
    if (result != MH_OK)
    {
      log::Error("Still failed to enable the hook");
      return;
    }
    else
      log::Ok("Hook enabled successfully, continuing...");
  }
  else
    createdHooks.insert(std::pair<std::string, __int64>(std::string(name), target));
}

//
// Hooks a function by changing the address at given index
// in the virtual function table
//
PBYTE WINAPI HookVTableFunction(PDWORD64* ppVTable, PBYTE pHook, SIZE_T iIndex)
{
  DWORD dwOld = 0;
  VirtualProtect((void*)((*ppVTable) + iIndex), sizeof(PDWORD64), PAGE_EXECUTE_READWRITE, &dwOld);

  PBYTE pOrig = ((PBYTE)(*ppVTable)[iIndex]);
  (*ppVTable)[iIndex] = (DWORD64)pHook;

  VirtualProtect((void*)((*ppVTable) + iIndex), sizeof(PDWORD64), dwOld, &dwOld);

  return pOrig;
}

IDXGISwapChain* pSwapChain = nullptr;

void hooks::Init()
{
  log::Write("Initializing hooks...");

  MH_STATUS status = MH_Initialize();
  if (status != MH_OK)
  {
    log::Error("Could not initialize MinHook. MH_STATUS 0x%X", status);
    return;
  }

  CreateHook("ComponentIterator", (__int64)FC::FCHandle + 0x79BEF30, hComponentIterator, &oComponentIterator);
  CreateHook("CameraUpdate", (__int64)FC::FCHandle + 0x1E4A450, hCameraUpdate, &oCameraUpdate);
  CreateHook("CameraAngles", (__int64)FC::FCHandle + 0xA6DD380, hCameraAngles, &oCameraAngles);
  CreateHook("GamepadUpdate", (__int64)FC::FCHandle + 0x6287970, hGamepadUpdate, &oGamepadUpdate);
  CreateHook("InputUpdate", (__int64)FC::FCHandle + 0x7EA52E0, hInputUpdate, &oInputUpdate);
  CreateHook("OnResize", (__int64)FC::FCHandle + 0x6512D20, hOnResize, &oOnResize);
  CreateHook("DrawFireUI", (__int64)FC::FCHandle + 0x66C2420, hDrawFireUI, &oDrawFireUI);

  HMODULE hUser32 = GetModuleHandleA("user32.dll");

  FARPROC pSetCursor = GetProcAddress(hUser32, "SetCursor");
  CreateHook("SetCursor", (__int64)pSetCursor, hSetCursor, &oSetCursor);

  FARPROC pSetCursorPos = GetProcAddress(hUser32, "SetCursorPos");
  CreateHook("SetCursorPos", (__int64)pSetCursorPos, hSetCursorPos, &oSetCursorPos);

  pSwapChain = FC::GetSwapChain();
  if (pSwapChain == nullptr)
  {
    log::Error("Failed to get IDXGISwapChain");
    return;
  }
  oD3D11Present = (tD3D11Present)HookVTableFunction((PDWORD64*)pSwapChain, (PBYTE)hD3D11Present, 8);

  log::Ok("Hooks initialized");
}

void hooks::DisableHooks()
{
//   for (auto itr = createdHooks.begin(); itr != createdHooks.end(); ++itr)
//   {
//     MH_STATUS status = MH_DisableHook((LPVOID)itr->second);
//     if (status != MH_OK)
//       util::log::Error("Could not disable \"%s\" hook. MH_STATUS 0x%X", itr->first.c_str(), status);
//   }
  HookVTableFunction((PDWORD64*)pSwapChain, (PBYTE)oD3D11Present, 8);
}

void hooks::EnableHooks()
{
  //MH_STATUS status = MH_EnableHook(MH_ALL_HOOKS);
  //if (status != MH_OK)
  //  log::Error("Could not enable all hooks. MH_STATUS 0x%X", status);
  pSwapChain = FC::GetSwapChain();
  (tD3D11Present)HookVTableFunction((PDWORD64*)pSwapChain, (PBYTE)hD3D11Present, 8);
}

void hooks::RemoveHooks()
{
  hooks::DisableHooks();

  for (auto itr = createdHooks.begin(); itr != createdHooks.end(); ++itr)
  {
    MH_STATUS status = MH_RemoveHook((LPVOID)itr->second);
    if (status != MH_OK)
      util::log::Error("Could not remove \"%s\" hook. MH_STATUS 0x%X", itr->first.c_str(), status);
  }
}

void hooks::Uninitialize()
{
  hooks::RemoveHooks();
  MH_STATUS status = MH_Uninitialize();
  if (status != MH_OK)
    log::Error("Could not uninitialize MinHook. MH_STATUS 0x%X", status);
}
