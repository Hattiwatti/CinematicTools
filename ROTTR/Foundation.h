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
    // TODO: Figure it out because entity variables are not enough
  public:
    // Creates a light resource with empty data
    static CommonLightResource* Create()
    {
      char dummyData[0x350] = { 0 };

      typedef CommonLightResource*(__fastcall* tCreateLightResource)(__int64, void*);
      tCreateLightResource CreateLightResource = (tCreateLightResource)0x14382E080;

      return CreateLightResource(0, dummyData);
    }
  }; // Size = 0x4A0

  class CommonRenderLight
  {
  public:
    char Pad000[0x30];
    SceneLight* m_SceneLight;
    CommonLightResource* m_LightResource;
    char Pad040[0x20];
    DirectX::XMMATRIX m_Transform;  // 0x60
    DirectX::XMFLOAT4 m_Color;      // 0xA0
    float m_AttenuationDistance;    // 0xB0
    float m_Unk1;       // 0xB4 Loads of spotlight values
    float m_Unk2;       // 0xB8
    float m_Unk3;       // 0xBC
    char Pad0C0[0x24];  // 0xC0
    float m_Unk4;       // 0xE4
    float m_Intensity;  // 0xE8
    float m_Unk5;       // 0xEC
    char Pad0F0[0x4];   // 0xF0
    float m_Unk6;       // 0xF4
    char Pad0F8[0x48];  // 0xF8
    int m_Flag1;        // 0x140 Type? 0 for point, 1/2 for spot
    int m_Flag2;        // 0x144 Updateflags?
    char Pad148[0xB8];  // 0x148
  }; // Size = 0x200

  class LightRenderCallback
  {
  public:
    virtual void Func1();
    virtual void Func2();
    virtual void RenderLight(SceneLight*);

  public:
    static LightRenderCallback* Singleton()
    {
      return (LightRenderCallback*)(0x141070f90);
    }
  };

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

  struct SceneEntityData
  {
    DirectX::XMMATRIX Transform1{ DirectX::XMMatrixIdentity() }; // 0x0
    DirectX::XMMATRIX Transform2{ DirectX::XMMatrixIdentity() }; // 0x40
    char Pad080[0x64]{ 0 };
    int Flag1{ 0 };  // 0xE4 
    int Flag2{ 8 };  // 0xE8
    char Pad0EC[0x214]{ 0 };
  };

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
    virtual void Func15();
    virtual void Func16();
    virtual void Func17();
    virtual void Func18();
    virtual void Func19();
    virtual void Func20();
    virtual void Func21();
    virtual void Func22();
    virtual void SetEntityData(SceneEntityData*);
    virtual SceneEntityData* GetEntityData();

    char Pad008[0x18];
    CommonRenderLight* m_CommonRenderLight;
    char Pad028[0x298];

  }; // Size = 0x2C0 (Actual total size with SceneEntity = 0x530)
  
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