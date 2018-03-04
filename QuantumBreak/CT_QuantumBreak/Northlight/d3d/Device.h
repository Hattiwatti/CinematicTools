#pragma once
#include <d3d11.h>

namespace d3d
{
  class Device
  {
  public:
    BYTE Pad000[0x20];
    IDXGISwapChain* m_pSwapchain;
    BYTE Pad028[0x30];
    HWND m_hwnd;

    static ID3D11Device* GetDevice()
    {
      typedef ID3D11Device*(__fastcall* tGetDevice)();

      HMODULE hModule = GetModuleHandleA("d3d_x64_f.dll");
      tGetDevice oGetDevice = (tGetDevice)GetProcAddress(hModule, "?getDevice@Device@d3d@@QEAAPEAXXZ");

      return oGetDevice();
    }

    static ID3D11DeviceContext* GetContext()
    {
      typedef ID3D11DeviceContext*(__fastcall* tGetContext)();

      HMODULE hModule = GetModuleHandleA("d3d_x64_f.dll");
      tGetContext oGetContext = (tGetContext)GetProcAddress(hModule, "?getDeviceContext@Device@d3d@@QEAAPEAXXZ");

      return oGetContext();
    }

    static Device* Singleton()
    {
      typedef Device*(__fastcall* tGetInstance)();

      HMODULE hModule = GetModuleHandleA("d3d_x64_f.dll");
      tGetInstance GetInstance = (tGetInstance)GetProcAddress(hModule, "?getInstanceUnchecked@Device@d3d@@SAPEAV12@XZ");

      return GetInstance();
    }
  };
}