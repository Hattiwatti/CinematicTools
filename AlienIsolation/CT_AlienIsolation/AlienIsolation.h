#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

namespace AI
{
  namespace Rendering
  {
    static IDXGISwapChain* GetSwapChain()
    {
      int pClass = *(int*)((int)GetModuleHandleA("AI.exe") + 0x17DF5CC);
      return *(IDXGISwapChain**)(pClass + 0x8);
    }

    static ID3D11DeviceContext* GetContext()
    {
      return *(ID3D11DeviceContext**)((int)GetModuleHandleA("AI.exe") + 0x17FE3F0);
    }

    static ID3D11Device* GetDevice()
    {
      ID3D11DeviceContext* pContext = GetContext();
      ID3D11Device* pDevice = nullptr;
      pContext->GetDevice(&pDevice);
      return pDevice;
    }

    static HWND GetHwnd()
    {
      HWND* pHwnd = (HWND*)((int)GetModuleHandleA("AI.exe") + 0x1251288);
      return *pHwnd;
    }

    static bool HasFocus()
    {
      return true;
      bool* pFocus = (bool*)((int)GetModuleHandleA("AI.exe") + 0x1251298);
      return *pFocus;
    }
  }
}