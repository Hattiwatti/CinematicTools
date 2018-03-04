#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

namespace AI
{
	class D3D
	{
	public:
		BYTE Pad000[0x4];
    ID3D11Device* m_pDevice;
		IDXGISwapChain* m_pSwapChain;

	public:
		static D3D* Singleton() 
    { 
      return *(D3D**)((int)GetModuleHandleA("AI.exe") + 0x17DF5CC); 
    }
	};
}