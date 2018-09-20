#pragma once
#include <d3d11.h>
#include "Util/Util.h"

namespace Foundation
{
  class GameRender;
  class CommonLightResource;
  class CommonRenderLight;
  class Scene;
  class SceneLight;

  class PCDX11DeviceManager;
  class PCDX11SwapChain;
  class PCDX11RenderDevice;

  class GameRender
  {
  public:
    char Pad000[0x38];
    float m_FieldOfView;        // 0x38
    char Pad03C[0x24];
    DirectX::XMMATRIX m_CameraTransform; // 0x60
    DirectX::XMMATRIX m_PrevTransform;   // 0xA0
    char Pad0E0[0xCA80];

  public:
    static GameRender* Singleton()
    {
      return *(GameRender**)(util::offsets::GetOffset("OFFSET_GAMERENDER"));
    }
  }; // Size = 0xCB60

  class CommonLightResource
  {
  public:
    char Pad000[0x4A0];
  }; // Size = 0x4A0

  class CommonRenderLight
  {
  public:
    char Pad000[0x200];
  }; // Size = 0x200

  class Scene
  {
  public:
    virtual void Func1();
    virtual void Func2();
    virtual void Func3();
    virtual void Func4();
    virtual void Func5();
    virtual void Func6();
    virtual void Func7();
    virtual void Func8();
    virtual void Func9();
    virtual void Func10();
    virtual void Func11();
    virtual void Func12();
    virtual void Func13();
    virtual void Func14();
    virtual SceneLight* CreateLight(CommonLightResource* pResource);

    char Pad008[0x3D58];

  public:
    static Scene* Singleton()
    {
      return *(Scene**)(util::offsets::GetOffset("OFFSET_SCENE"));
    }
  }; // Size = 0x3D60

  class SceneEntity
  {
  public:
    virtual void Func1();
    
    char Pad008[0x268];
  }; // Size = 0x270

  class SceneLight
  {
  public:
    virtual void Func1();
    char Pad008[0x18];
    CommonRenderLight* m_CommonRenderLight;
    char Pad028[0x298];

  }; // Size = 0x2C0
  
  class PCDX11DeviceManager
  {
  public:
    char Pad000[0x48];
    ID3D11Device* m_pD3D11Device;

  public:
    static PCDX11DeviceManager* Singleton()
    {
      return *(PCDX11DeviceManager**)(util::offsets::GetOffset("OFFSET_DX11DEVICEMANAGER"));
    }
  };

  class PCDX11RenderDevice
  {
  public:
    char Pad000[0x1FB78];
    PCDX11SwapChain* m_Dx11SwapChain;

  public:
    static PCDX11RenderDevice* Singleton()
    {
      return *(PCDX11RenderDevice**)(util::offsets::GetOffset("OFFSET_DX11RENDERDEVICE"));
    }
  };

  class PCDX11SwapChain
  {
  public:
    char Pad000[0x60];
    IDXGISwapChain* m_SwapChain;
  };
}