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
    static const unsigned char DefaultVoxelData[0xFF];

  public:
    char Pad000[0x120];
    DirectX::XMFLOAT4 m_Color;  // 0x120
    int m_Type;                 // 0x130
    char Pad134[0x4];           // 0x134
    float m_Unk1;               // 0x138 Default 1.0
    char Pad13C[0x1B];          // 0x13C
    unsigned char m_VoxelData[0xFF];  // 0x157
    char Pad256[0x32];          // 0x256
    int m_Unk2;                 // 0x288 Default 1
    char Pad28C[0x1A4];         // 0x28C
    DirectX::XMFLOAT4 m_Unk3;   // 0x430 Default 1 1 1 1, specular etc multipliers?
    float m_Unk4;               // 0x440 Default 1.0
    float m_Unk5;               // 0x444 Default 1.0
    char Pad448[0x58];          // 0x448
  public:
    // Creates a light resource with empty data
    static CommonLightResource* Create()
    {
      char dummyData[0x350] = { 0 };

      typedef CommonLightResource*(__fastcall* tCreateLightResource)(__int64, void*);
      tCreateLightResource CreateLightResource = (tCreateLightResource)0x14382E080;

      return CreateLightResource(0, dummyData);
    }

    void LoadDefaultValues()
    {
      m_Type = 1;
      m_Unk1 = 1.f;
      m_Unk2 = 1;
      m_Unk3 = { 1.0f, 1.0f, 1.0f, 1.0f };
      m_Unk4 = 1.0f;
      m_Unk5 = 1.0f;

      memcpy(&m_VoxelData[0], &DefaultVoxelData[0], 0xFF);
    }
  }; // Size = 0x4A0

  class CommonRenderLight
  {
  public:
    char Pad000[0x30];
    SceneLight* m_SceneLight;
    CommonLightResource* m_LightResource;
    float m_Unk1;                   // 0x40 Default 3.61
    float m_Unk2;                   // 0x44 Default -2.509
    char Pad048[0x10];              // 0x48
    float m_Unk3;                   // 0x58 Default 1.0f
    char Pad05C[0x4];               // 0x5C
    DirectX::XMMATRIX m_Transform;  // 0x60
    DirectX::XMFLOAT4 m_Color;      // 0xA0
    float m_AttenuationDistance;    // 0xB0
    float m_Unk4;       // 0xB4 Default 1.64f
    float m_Unk5;       // 0xB8 Default 0.61f
    float m_Unk6;       // 0xBC Default 0
    DirectX::XMFLOAT4 m_Unk7; // 0xC0 Default 2000 4000 8000 1.0f
    float m_Unk8;       // 0xD0 Default 2.20
    char Pad0D4[0xC];   // 0xD4
    float m_Unk9;       // 0xE0 Default 1.0
    float m_Unk10;      // 0xE4 Default 2.0
    float m_Intensity;  // 0xE8 Default 1.0
    float m_Unk11;      // 0xEC Default 1.0
    float m_Unk12;      // 0xF0 Default 0
    float m_Unk13;      // 0xF4 Default 1.07
    float m_Unk14;      // 0xF8 Default 0
    float m_Unk15;      // 0xFC Default 0
    DirectX::XMMATRIX m_Unk16; // 0x100 Default identity
    int m_Flag1;        // 0x140 Default 2
    int m_Flag2;        // 0x144 Default 0xC4
    char Pad148[0xB8];  // 0x148

  public:
    void LoadDefaultValues()
    {
      memset(&Pad048[0], 0, 0x10);
      memset(&Pad05C[0], 0, 0x4);
      memset(&Pad0D4[0], 0, 0xC);

      m_Unk1 = 3.61f;
      m_Unk2 = -2.509f;
      m_Unk3 = 1.0f;
      m_Unk4 = 1.64f;
      m_Unk5 = 0.61f;
      m_Unk6 = 0.f;
      m_Unk7 = { 2000.f, 4000.f, 8000.f, 1.0f };
      m_Unk8 = 2.20f;
      m_Unk9 = 1.0f;
      m_Unk10 = 2.0f;
      m_Intensity = 1.0f;
      m_Unk11 = 1.0f;
      m_Unk12 = 0.f;
      m_Unk13 = 1.07f;
      m_Unk14 = 0.f;
      m_Unk15 = 0.f;
      m_Unk16 = DirectX::XMMatrixIdentity();
      m_Flag1 = 2;
      m_Flag2 = 0xC4;
    }
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