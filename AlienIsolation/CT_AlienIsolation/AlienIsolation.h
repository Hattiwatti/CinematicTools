#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

namespace AI
{

  namespace Input
  {
    class Mouse
    {
    public:
      BYTE Pad000[0x10];
      BYTE m_enableGameInput; // 0 = shows cursor, stops any game input
      BYTE m_enableAxisInput; // 0 = allows button presses, but no camera rotation
      BYTE Pad012[0x2];
      float m_dAxis[6];
      XMFLOAT2 m_mouseCoordinates;
      BYTE Pad034[0x70];
      BYTE m_showCursor;
    };

    class Keyboard
    {
      BYTE Pad000[0x10];
      BYTE m_enableInput;
      BYTE Pad011[0x3];
      BYTE m_inputCache[256];
      BYTE m_inputCache2[256];
    };

    static Mouse* GetMouse()
    {
      //AI.exe + 1359B44
      int ptr1 = *(int*)((int)GetModuleHandleA("AI.exe") + 0x1359B44);
      return *(Mouse**)ptr1;
    }
  }

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