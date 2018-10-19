#include "Util.h"
#include "../Globals.h"
#include "../Foundation.h"

#include <MinHook.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <unordered_map>
#include <Windows.h>

#pragma comment(lib, "libMinHook.x64.lib")

// Function definitions
typedef DWORD(WINAPI* tIDXGISwapChain_Present)(IDXGISwapChain*, UINT, UINT);

typedef void(__fastcall* tCameraUpdate)(Foundation::GameRender*);
typedef __int64(__fastcall* tInputDeviceUpdate)(__int64, __int64, __int64, __int64);
typedef int(__fastcall* tScaleformRenderer_Render)(__int64);

//////////////////////////
////   RENDER HOOKS   ////
//////////////////////////

// Most games today run on DirectX 11. This hook is called just before
// the image is actually drawn on screen so we can draw more stuff
// on top, like the tools UI.

tIDXGISwapChain_Present oIDXGISwapChain_Present = nullptr;
tScaleformRenderer_Render oScaleformRenderer_Render = nullptr;

DWORD WINAPI hIDXGISwapChain_Present(IDXGISwapChain* pSwapchain, UINT SyncInterval, UINT Flags)
{
  if (!g_shutdown)
  {
    CTRenderer* pRenderer = g_mainHandle->GetRenderer();
    pRenderer->BindUIRenderTarget();
    g_mainHandle->GetUI()->Draw();
  }

  return oIDXGISwapChain_Present(pSwapchain, SyncInterval, Flags);
}

void __fastcall hScaleformRenderer_Render(__int64 a1)
{
  if (g_mainHandle->GetCameraManager()->IsHudDisabled())
    return;

  oScaleformRenderer_Render(a1);
}

//////////////////////////
////   CAMERA HOOKS   ////
//////////////////////////

tCameraUpdate oCameraUpdate = nullptr;

void __fastcall hCameraUpdate(Foundation::GameRender* a1)
{
  oCameraUpdate(a1);
  g_mainHandle->GetCameraManager()->OnCameraUpdate(a1);
}


//////////////////////////
////   INPUT HOOKS    ////
//////////////////////////

tInputDeviceUpdate oGamepadUpdate = nullptr;
tInputDeviceUpdate oKeyboardMouseUpdate = nullptr;

__int64 __fastcall hGamepadUpdate(__int64 a1, __int64 a2, __int64 a3, __int64 a4)
{
  CameraManager* pCameraManager = g_mainHandle->GetCameraManager();
  if (pCameraManager->IsCameraEnabled() && pCameraManager->IsGamepadDisabled())
    return 0x14;

  return oGamepadUpdate(a1, a2, a3, a4);
}

__int64 __fastcall hKeyboardMouseUpdate(__int64 a1, __int64 a2, __int64 a3, __int64 a4)
{
  CameraManager* pCameraManager = g_mainHandle->GetCameraManager();
  if (pCameraManager->IsCameraEnabled() && pCameraManager->IsKbmDisabled())
    return 0x36;

  return oKeyboardMouseUpdate(a1, a2, a3, a4);
}


//////////////////////////
////   OTHER HOOKS    ////
//////////////////////////

typedef void(__fastcall* tSceneRenderLights)(__int64, __int64);
tSceneRenderLights oSceneRenderLights = nullptr;

void __fastcall hSceneRenderLights(__int64 a1, __int64 a2)
{
  oSceneRenderLights(a1, a2);
  //g_mainHandle->RenderTestLight();
}


/*----------------------------------------------------------------*/

namespace
{
  std::unordered_map<std::string, util::hooks::Hook> m_CreatedHooks;
}

// Creates a normal function hook with MinHook, 
// which places a jmp instruction at the start of the function.
template <typename T>
static void CreateHook(std::string const& name, __int64 target, PVOID hook, T original)
{
  LPVOID* pOriginal = reinterpret_cast<LPVOID*>(original);
  MH_STATUS result = MH_CreateHook((LPVOID)target, hook, pOriginal);
  if (result != MH_OK)
  {
    util::log::Error("Could not create %s hook. MH_STATUS 0x%X error code 0x%X", name, result, GetLastError());
    return;
  }

  result = MH_EnableHook((LPVOID)target);
  if (result != MH_OK)
  {
    util::log::Error("Could not enable %s hook. MH_STATUS 0x%X error code 0x%X", name, result, GetLastError());
    return;
  }

  util::hooks::Hook hookInfo{ 0 };
  hookInfo.Address = target;
  hookInfo.Type = util::hooks::HookType::MinHook;
  hookInfo.Enabled = true;

  m_CreatedHooks.emplace(name, hookInfo);
}

// Write to VTable
static PBYTE WINAPI WriteToVTable(PDWORD64* ppVTable, PVOID hook, SIZE_T iIndex)
{
  DWORD dwOld = 0;
  VirtualProtect((void*)((*ppVTable) + iIndex), sizeof(PDWORD64), PAGE_EXECUTE_READWRITE, &dwOld);

  PBYTE pOrig = ((PBYTE)(*ppVTable)[iIndex]);
  (*ppVTable)[iIndex] = (DWORD64)hook;

  VirtualProtect((void*)((*ppVTable) + iIndex), sizeof(PDWORD64), dwOld, &dwOld);
  return pOrig;
}

// Hooks a function by changing the address at given index
// in the virtual function table.
template <typename T>
static void CreateVTableHook(std::string const& name, PDWORD64* ppVTable, PVOID hook, SIZE_T iIndex, T original)
{
  LPVOID* pOriginal = reinterpret_cast<LPVOID*>(original);
  *pOriginal = reinterpret_cast<LPVOID>(WriteToVTable(ppVTable, hook, iIndex));

  util::hooks::Hook hookInfo{ 0 };
  hookInfo.Address = (__int64)ppVTable;
  hookInfo.Index = iIndex;
  hookInfo.Type = util::hooks::HookType::VTable;
  hookInfo.Original = pOriginal;
  hookInfo.Enabled = true;

  m_CreatedHooks.emplace(name, hookInfo);
}

void util::hooks::Init()
{
  MH_STATUS status = MH_Initialize();
  if (status != MH_OK)
    util::log::Error("Failed to initialize MinHook, MH_STATUS 0x%X", status);

  CreateHook("CameraUpdate", util::offsets::GetOffset("OFFSET_CAMERAUPDATE"), hCameraUpdate, &oCameraUpdate);

  CreateHook("KeyboardMouseUpdate", util::offsets::GetOffset("OFFSET_KEYBOARDMOUSEUPDATE"), hKeyboardMouseUpdate, &oKeyboardMouseUpdate);
  CreateHook("GamepadUpdate", util::offsets::GetOffset("OFFSET_GAMEPADUPDATE"), hGamepadUpdate, &oGamepadUpdate);
  //CreateHook("SceneLightRender", 0x1439D50B0, hSceneRenderLights, &oSceneRenderLights);
  CreateHook("ScaleformRender", util::offsets::GetOffset("OFFSET_SCALEFORMRENDER"), hScaleformRenderer_Render, &oScaleformRenderer_Render);

  CreateVTableHook("SwapChainPresent", (PDWORD64*)g_dxgiSwapChain, hIDXGISwapChain_Present, 8, &oIDXGISwapChain_Present);
}

// In some cases it's useful or even required to disable all hooks or just certain ones
void util::hooks::SetHookState(bool enable, std::string const& name)
{
  if (name.empty())
  {
    MH_STATUS status = enable ? MH_EnableHook(MH_ALL_HOOKS) : MH_DisableHook(MH_ALL_HOOKS);
    if (status != MH_OK)
      util::log::Error("MinHook failed to %s all hooks, MH_STATUS 0x%X", (enable ? "enable" : "disable"), status);

    for (auto& entry : m_CreatedHooks)
    {
      Hook& hook = entry.second;
      if (hook.Type == HookType::MinHook) continue;
      if (hook.Enabled != enable)
      {
        *hook.Original = WriteToVTable((PDWORD64*)hook.Address, *hook.Original, hook.Index);
        hook.Enabled = enable;
      }
      else
        util::log::Warning("VTable hook %s is already %s", name.c_str(), enable ? "enabled" : "disabled");
    }
  }
  else
  {
    auto result = m_CreatedHooks.find(name);
    if (result == m_CreatedHooks.end())
    {
      util::log::Error("Hook %s does not exit", name.c_str());
      return;
    }

    Hook& hook = result->second;
    if (hook.Type == HookType::MinHook)
    {
      MH_STATUS status = enable ? MH_EnableHook((LPVOID)hook.Address) : MH_DisableHook((LPVOID)hook.Address);
      if (status != MH_OK)
        util::log::Error("MinHook failed to %s hook %s, MH_STATUS 0x%X", (enable ? "enable" : "disable"), name.c_str(), status);
    }
    else
    {
      if (hook.Enabled != enable)
      {
        *hook.Original = WriteToVTable((PDWORD64*)hook.Address, *hook.Original, hook.Index);
        hook.Enabled = enable;
      }
      else
        util::log::Warning("VTable hook %s is already %s", name.c_str(), enable ? "enabled" : "disabled");
    }
  }
}
