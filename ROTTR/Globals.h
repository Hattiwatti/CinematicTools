#pragma once
#include "Main.h"
#include <d3d11.h>
#include <Windows.h>

extern bool g_shutdown;
extern bool g_hasFocus;

extern Main* g_mainHandle;
extern HINSTANCE g_dllHandle;

extern HINSTANCE g_gameHandle;
extern HWND g_gameHwnd;
extern WNDPROC g_origWndProc;

extern ID3D11Device* g_d3d11Device;
extern ID3D11DeviceContext* g_d3d11Context;
extern IDXGISwapChain* g_dxgiSwapChain;