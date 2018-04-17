#include "Util.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <Windows.h>

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
  // Draw UI
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

// Creates a normal function hook with MinHook, 
// which places a jmp instruction at the start of the function.
template <typename T>
static void CreateHook(const char* name, __int64 target, PVOID hook, T original)
{
  /*
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
    */
}

// Hooks a function by changing the address at given index
// in the virtual function table.
static PBYTE WINAPI HookVTableFunction(PDWORD64* ppVTable, PBYTE pHook, SIZE_T iIndex)
{
  DWORD dwOld = 0;
  VirtualProtect((void*)((*ppVTable) + iIndex), sizeof(PDWORD64), PAGE_EXECUTE_READWRITE, &dwOld);

  PBYTE pOrig = ((PBYTE)(*ppVTable)[iIndex]);
  (*ppVTable)[iIndex] = (DWORD64)pHook;

  VirtualProtect((void*)((*ppVTable) + iIndex), sizeof(PDWORD64), dwOld, &dwOld);
  return pOrig;
}

void util::hooks::Init()
{

  IDXGISwapChain* pSwapChain = 0x0; // Find the game's IDXGISwapChain object and hook it
  oIDXGISwapChain_Present = (tIDXGISwapChain_Present)HookVTableFunction((PDWORD64*)pSwapChain, (PBYTE)hIDXGISwapChain_Present, 8);
}