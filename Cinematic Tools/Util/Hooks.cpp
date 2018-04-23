#include "Util.h"
#include "../Main.h"

#include <MinHook.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <unordered_map>
#include <Windows.h>

#pragma comment(lib, "libMinHook.x64.lib")

// Function definitions
typedef DWORD(WINAPI* tIDXGISwapChain_Present)(IDXGISwapChain*, UINT, UINT);
typedef __int64(__fastcall* tCameraUpdateExample)(__int64, DirectX::XMFLOAT4X4*);

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

tCameraUpdateExample oCameraUpdateExample = nullptr;
__int64 __fastcall hCameraUpdateExample(__int64 pCamera, DirectX::XMFLOAT4X4* pMatrix)
{
  // In this example the function could be an update function of the 
  // camera object, that takes a new transform matrix as an argument.
  // This is the transform we want to override with our own camera.

  // if(g_mainHandle->GetCameraManager()->IsCameraEnabled())
  // {
  //    XMFLOAT4X4& customTransform = g_mainHandle->GetCameraManager()->GetCameraTransform();
  //    *pMatrix = customTransform;
  //    return 0;
  // }

  return oCameraUpdateExample(pCamera, pMatrix);
}


//////////////////////////
////   INPUT HOOKS    ////
//////////////////////////

// I like to try and provide a way to still move your character
// while having the camera enabled. This usually works by finding
// a function that specifically controls your character's input.
// Useful when you want to record a video and be your own actor.
// CameraManager provides IsKbmDisabled() and IsGamepadDisabled()

//////////////////////////
////   OTHER HOOKS    ////
//////////////////////////

// These could be hooks on AI to disable them, object iterators
// to find certain components, something to disable the usual
// outlines on friendly players/characters etc...
// Hooks on cursor functions are usually needed to show the mouse
// when tools UI is open.


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
    log::Error("Could not create %s hook. MH_STATUS 0x%X error code 0x%X", name, result, GetLastError());
    return;
  }

  result = MH_EnableHook((LPVOID)target);
  if (result != MH_OK)
  {
    log::Error("Could not enable %s hook. MH_STATUS 0x%X error code 0x%X", name, result, GetLastError());
    return;
  }

  Hook hookInfo{ 0 };
  hookInfo.Address = target;
  hookInfo.Type = HookType::MinHook;

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
  *original = WriteToVTable(ppVTable, hook, iIndex);

  Hook hookInfo{ 0 };
  hookInfo.Address = ppVTable;
  hookInfo.Index = iIndex;
  hookInfo.Type = HookType::VTable;
  hookInfo.Original = pOriginal;

  m_CreatedHooks.emplace(name, hookInfo);
}

void util::hooks::Init()
{
  MH_STATUS status = MH_Initialize();
  if (status != MH_OK)
    util::log::Error("Failed to initialize MinHook, MH_STATUS 0x%X", status);

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
