#include "Util.h"
#include "../Main.h"

#include "../AlienIsolation.h"
#include <MinHook.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <unordered_map>
#include <Windows.h>

#pragma comment(lib, "libMinHook.x86.lib")

// Function definitions
typedef DWORD(WINAPI* tIDXGISwapChain_Present)(IDXGISwapChain*, UINT, UINT);
typedef BOOL(WINAPI* tSetCursorPos)(int, int);

typedef int(__thiscall* tCameraUpdate)(CATHODE::AICameraManager*);
typedef int(__thiscall* tInputUpdate)(void*);
typedef int(__thiscall* tGamepadUpdate)(void*);

typedef int(__thiscall* tPostProcessUpdate)(int);
typedef char(__stdcall* tTonemapUpdate)(CATHODE::DayToneMapSettings*, int);
typedef bool(__thiscall* tCombatManagerUpdate)(void*, CATHODE::Character*);

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
  {
    g_mainHandle->GetUI()->BindRenderTarget();
    g_mainHandle->GetRenderer()->UpdateMatrices();
    //g_mainHandle->GetCameraManager()->DrawTrack();

    g_mainHandle->GetUI()->Draw();
  }

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

int __fastcall hCameraUpdate(CATHODE::AICameraManager* pCameraManager)
{
  g_mainHandle->GetCameraManager()->OnCameraUpdateBegin();
  int result = oCameraUpdate(pCameraManager);
  g_mainHandle->GetCameraManager()->OnCameraUpdateEnd();
  return result;
}


//////////////////////////
////   INPUT HOOKS    ////
//////////////////////////

tInputUpdate oInputUpdate = nullptr;
tGamepadUpdate oGamepadUpdate = nullptr;
tSetCursorPos oSetCursorPos = nullptr;

int __fastcall hInputUpdate(void* _this)
{
  CameraManager* pCameraManager = g_mainHandle->GetCameraManager();
  if (pCameraManager->IsCameraEnabled() && pCameraManager->IsKbmDisabled())
    return 0;

  return oInputUpdate(_this);
}

int __fastcall hGamepadUpdate(void* _this)
{
  CameraManager* pCameraManager = g_mainHandle->GetCameraManager();
  InputSystem* pInputSystem = g_mainHandle->GetInputSystem();

  if (pCameraManager->IsCameraEnabled()
    && pCameraManager->IsGamepadDisabled()
    && !pInputSystem->IsUsingSecondPad())
    return 0;
  
  return oGamepadUpdate(_this);
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

// These could be hooks on AI to disable them, object iterators
// to find certain components, something to disable the usual
// outlines on friendly players/characters etc...
// Hooks on cursor functions are usually needed to show the mouse
// when tools UI is open.

tPostProcessUpdate oPostProcessUpdate = nullptr;
tCombatManagerUpdate oCombatManagerUpdate = nullptr;
tTonemapUpdate oTonemapUpdate = nullptr;

int __fastcall hPostProcessUpdate(int _this)
{
  int result = oPostProcessUpdate(_this);

  CATHODE::PostProcess* pPostProcess = reinterpret_cast<CATHODE::PostProcess*>(_this + 0x1918);
  g_mainHandle->GetCameraManager()->OnPostProcessUpdate(pPostProcess);
  g_mainHandle->GetVisualsController()->OnPostProcessUpdate(pPostProcess);
  return result;
}

bool __fastcall hCombatManagerUpdate(void* _this, void* _EDX, CATHODE::Character* pTargetChr)
{
  if (g_mainHandle->GetCharacterController()->IsPlayerInvisible())
  {
    CATHODE::Character* pPlayer = CATHODE::Main::Singleton()->m_CharacterManager->m_PlayerCharacters[0];
    if (pPlayer == pTargetChr)
      return false;
  }

  return oCombatManagerUpdate(_this, pTargetChr);
}

char __stdcall hTonemapSettings(CATHODE::DayToneMapSettings* pTonemapSettings, int a2)
{
  char result = oTonemapUpdate(pTonemapSettings, a2);
  g_mainHandle->GetVisualsController()->OnTonemapUpdate();

  return result;
}


/*----------------------------------------------------------------*/

namespace
{
  std::unordered_map<std::string, util::hooks::Hook> m_CreatedHooks;
}

// Creates a normal function hook with MinHook, 
// which places a jmp instruction at the start of the function.
template <typename T>
static void CreateHook(std::string const& name, int target, PVOID hook, T original)
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
static PBYTE WINAPI WriteToVTable(PDWORD* ppVTable, PVOID hook, SIZE_T iIndex)
{
  DWORD dwOld = 0;
  VirtualProtect((void*)((*ppVTable) + iIndex), sizeof(PDWORD), PAGE_EXECUTE_READWRITE, &dwOld);

  PBYTE pOrig = ((PBYTE)(*ppVTable)[iIndex]);
  (*ppVTable)[iIndex] = (DWORD)hook;

  VirtualProtect((void*)((*ppVTable) + iIndex), sizeof(PDWORD), dwOld, &dwOld);
  return pOrig;
}

// Hooks a function by changing the address at given index
// in the virtual function table.
template <typename T>
static void CreateVTableHook(std::string const& name, PDWORD* ppVTable, PVOID hook, SIZE_T iIndex, T original)
{
  LPVOID* pOriginal = reinterpret_cast<LPVOID*>(original);
  *pOriginal = reinterpret_cast<LPVOID>(WriteToVTable(ppVTable, hook, iIndex));

  util::hooks::Hook hookInfo{ 0 };
  hookInfo.Address = (int)ppVTable;
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
  //CreateHook("InputUpdate", util::offsets::GetOffset("OFFSET_INPUTUPDATE"), hInputUpdate, &oInputUpdate);
  CreateHook("GamepadUpdate", util::offsets::GetOffset("OFFSET_GAMEPADUPDATE"), hGamepadUpdate, &oGamepadUpdate);
  CreateHook("PostProcessUpdate", util::offsets::GetOffset("OFFSET_POSTPROCESSUPDATE"), hPostProcessUpdate, &oPostProcessUpdate);
  CreateHook("TonemapUpdate", util::offsets::GetOffset("OFFSET_TONEMAPUPDATE"), hTonemapSettings, &oTonemapUpdate);
  //CreateHook("AICombatManagerUpdate", util::offsets::GetOffset("OFFSET_COMBATMANAGERUPDATE"), hCombatManagerUpdate, &oCombatManagerUpdate);
  
  CreateHook("SetCursorPos", (int)GetProcAddress(GetModuleHandleA("user32.dll"), "SetCursorPos"), hSetCursorPos, &oSetCursorPos);

  CreateVTableHook("SwapChainPresent", (PDWORD*)g_dxgiSwapChain, hIDXGISwapChain_Present, 8, &oIDXGISwapChain_Present);
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
        *hook.Original = WriteToVTable((PDWORD*)hook.Address, *hook.Original, hook.Index);
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
        *hook.Original = WriteToVTable((PDWORD*)hook.Address, *hook.Original, hook.Index);
        hook.Enabled = enable;
      }
      else
        util::log::Warning("VTable hook %s is already %s", name.c_str(), enable ? "enabled" : "disabled");
    }
  }
}
