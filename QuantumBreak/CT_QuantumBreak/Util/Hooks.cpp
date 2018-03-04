#include "util.h"
#include "../Northlight/d3d/Device.h"
#include "../Main.h"
#include <MinHook.h>

#pragma comment(lib, "libMinHook.x64.lib")

using namespace util;

////////////////////////////////
////  FUNCTION DEFINITIONS  ////
////////////////////////////////

typedef HRESULT(WINAPI * tD3D11Present)(IDXGISwapChain*, UINT, UINT);

////////////////////////////////
////  RENDER-RELATED HOOKS  ////
////////////////////////////////

tD3D11Present oD3D11Present = nullptr;

HRESULT WINAPI hD3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
  if (!g_shutdown)
  {
    g_mainHandle->GetUI()->Draw();
  }

  return oD3D11Present(pSwapChain, SyncInterval, Flags);
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

  LPVOID address = reinterpret_cast<LPVOID>(target);
  MH_STATUS result = MH_CreateHook(address, hook, pOriginal);
  if (result != MH_OK)
  {
    log::Error("Could not create %s hook at 0x%I64X. MH_STATUS 0x%X error code 0x%X", name, address, result, GetLastError());
    return;
  }
  result = MH_EnableHook(address);
  if (result != MH_OK)
    log::Error("Could not enable %s hook. MH_STATUS 0x%X error code 0x%X", name, result, GetLastError());
  else
    createdHooks.insert(std::pair<std::string, __int64>(std::string(name), target));
}

void hooks::Init()
{
  log::Write("Initializing hooks...");

  MH_STATUS status = MH_Initialize();
  if (status != MH_OK)
  {
    log::Error("Could not initialize MinHook. MH_STATUS 0x%X", status);
    return;
  }

  oD3D11Present = (tD3D11Present)HookVTableFunction((PDWORD64*)d3d::Device::Singleton()->m_pSwapchain, (PBYTE)hD3D11Present, 8);
  log::Ok("Hooks initialized");
}

void hooks::DisableHooks()
{
  HookVTableFunction((PDWORD64*)d3d::Device::Singleton()->m_pSwapchain, (PBYTE)oD3D11Present, 8);
  MH_DisableHook(MH_ALL_HOOKS);
}

void hooks::EnableHooks()
{
  MH_STATUS status = MH_EnableHook(MH_ALL_HOOKS);
  if (status != MH_OK)
    log::Error("Could not enable all hooks. MH_STATUS 0x%X", status);

  HookVTableFunction((PDWORD64*)d3d::Device::Singleton()->m_pSwapchain, (PBYTE)hD3D11Present, 8);
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
