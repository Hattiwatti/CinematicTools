#include "Util.h"
#include "../Main.h"
#include "../Northlight.h"

#include <MinHook.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <unordered_map>
#include <Windows.h>

#pragma comment(lib, "libMinHook.x64.lib")

using namespace DirectX;

// Function definitions
typedef DWORD(WINAPI* tIDXGISwapChain_Present)(IDXGISwapChain*, UINT, UINT);
typedef BOOL(WINAPI* tSetCursorPos)(int x, int y);

typedef int(__fastcall* tCameraUpdate)(__int64, __int64, __int64, __int64);
typedef __int64(__fastcall* tAspectRatioHook)(__int64);
typedef void(__fastcall* tSetFov)(__int64, float);

typedef __int64(__fastcall* tGetXinputState)(int, __int64);
typedef void(__fastcall* tFreezeGame)(bool);


//////////////////////////
////   RENDER HOOKS   ////
//////////////////////////

// Most games today run on DirectX 11. This hook is called just before
// the image is actually drawn on screen so we can draw more stuff
// on top, like the tools UI.

tIDXGISwapChain_Present oIDXGISwapChain_Present = nullptr;
DWORD WINAPI hIDXGISwapChain_Present(IDXGISwapChain* pSwapchain, UINT SyncInterval, UINT Flags)
{
  if (!g_shutdown)
    g_mainHandle->GetUI()->Draw();

  return oIDXGISwapChain_Present(pSwapchain, SyncInterval, Flags);
}

//////////////////////////
////   CAMERA HOOKS   ////
//////////////////////////

// This is an example of what a camera hook could look like.
// Different games and engines have different kinds of methods
// and it's up to you to figure out what to hook and how to
// override the game's camera. Some games might require multiple
// hooks.

tCameraUpdate oCameraUpdate = nullptr;
tSetFov oSetFov = nullptr;
tAspectRatioHook oAspectRatioHook = nullptr;

int __fastcall hCameraUpdate(__int64 a1, __int64 a2, __int64 a3, __int64 a4)
{
  XMFLOAT4X3* gameCamera = reinterpret_cast<XMFLOAT4X3*>(a3);
  if (g_mainHandle->GetCameraManager()->IsCameraEnabled())
  {
    XMStoreFloat4x3(gameCamera, g_mainHandle->GetCameraManager()->GetMatrix());
  }
  else
  {
    XMMATRIX gameCameraMatrix = XMLoadFloat4x3(gameCamera);
    XMFLOAT4X4 resetMatrix;
    XMStoreFloat4x4(&resetMatrix, gameCameraMatrix);
    g_mainHandle->GetCameraManager()->SetResetMatrix(resetMatrix);
  }

  return oCameraUpdate(a1, a2, a3, a4);
}

void __fastcall hSetFov(__int64 a1, float a2)
{
  if (g_mainHandle->GetCameraManager()->IsCameraEnabled())
    return oSetFov(a1, g_mainHandle->GetCameraManager()->GetCameraFov());

  return oSetFov(a1, a2);
}

__int64 __fastcall hAspectRatioHook(__int64 a1)
{
  float* pOrigAspectRatio = reinterpret_cast<float*>(a1 + 0x2E4);

  if (g_mainHandle->GetCameraManager()->IsCameraEnabled() &&
    g_mainHandle->GetCameraManager()->OverrideRatio())
  {
    *pOrigAspectRatio = g_mainHandle->GetCameraManager()->GetAspectRatio();
  }

  return oAspectRatioHook(a1);
}

//////////////////////////
////   INPUT HOOKS    ////
//////////////////////////

tGetXinputState oGetXinputState = nullptr;
tSetCursorPos oSetCursorPos = nullptr;

__int64 __fastcall hGetXinputState(int a1, __int64 a2)
{
  if (g_mainHandle->GetCameraManager()->IsCameraEnabled() && 
      g_mainHandle->GetCameraManager()->IsGamepadDisabled())
    return 0;
  else
    return oGetXinputState(a1, a2);
}

BOOL WINAPI hSetCursorPos(int x, int y)
{
  if (g_mainHandle->GetUI()->IsEnabled())
    return TRUE;

  return oSetCursorPos(x, y);
}

//////////////////////////
////   OTHER HOOKS    ////
//////////////////////////

tFreezeGame oFreezeGame = nullptr;

// Don't allow unfreeze if time freeze is enabled (going to menu and back to game)
void __fastcall hFreezeGame(bool a1)
{
  CameraManager* pCameraManager = g_mainHandle->GetCameraManager();
  if (pCameraManager->IsTimeFrozen())
    return oFreezeGame(true);

  return oFreezeGame(a1);
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

  m_CreatedHooks.emplace(name, hookInfo);
}

void util::hooks::Init()
{
  MH_STATUS status = MH_Initialize();
  if (status != MH_OK)
    util::log::Error("Failed to initialize MinHook, MH_STATUS 0x%X", status);

  __int64 CameraUpdate = (__int64)g_gameHandle + 0x3A1FC0;
  __int64 SetFov = (__int64)GetProcAddress(g_rlModule, "?setFov@PerspectiveView@m@@QEAAXM@Z");
  __int64 GetXinputState = (__int64)GetProcAddress(g_rlModule, "?rmdXInputGetState@@YAKKPEAU_rmd_XINPUT_STATE@@@Z");
  __int64 AspectRatioHook = (__int64)g_rlModule + 0xA4D20;
  __int64 FreezeHook = (__int64)g_gameHandle + 0x357610;

  CreateVTableHook("SwapChainPresent", (PDWORD64*)g_dxgiSwapChain, hIDXGISwapChain_Present, 8, &oIDXGISwapChain_Present);
  CreateHook("CameraUpdate", CameraUpdate, hCameraUpdate, &oCameraUpdate);
  CreateHook("CameraFoV", SetFov, hSetFov, &oSetFov);
  CreateHook("GetXinputState", GetXinputState, hGetXinputState, &oGetXinputState);
  CreateHook("AspectRatioHook", AspectRatioHook, hAspectRatioHook, &oAspectRatioHook);
  CreateHook("FreezeHook", FreezeHook, hFreezeGame, &oFreezeGame);

  HMODULE hUser32 = GetModuleHandleA("user32.dll");
  FARPROC pSetCursorPos = GetProcAddress(hUser32, "SetCursorPos");
  CreateHook("SetCursorPos", (__int64)pSetCursorPos, hSetCursorPos, &oSetCursorPos);

  util::log::Ok("Hooks initialized");
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
