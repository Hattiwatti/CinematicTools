#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <Windows.h>

namespace FC
{
  extern HINSTANCE FCHandle;
  extern HWND FCHwnd;

  class CEnvironmentSettings;

  template <typename T>
  struct ComponentCollection
  {
    T* pComponents;
    unsigned int capacity;
    unsigned int size;
  };

  class CAIKnowledge
  {
  public:
    BYTE Pad000[0x7F];
    BYTE m_ZombieAI;

  public:
    static CAIKnowledge* Singleton()
    {
      return *(CAIKnowledge**)((__int64)FCHandle + 0x4D95378);
    }
  };

  class CDynamicEnvironment
  {
  public:
    BYTE Pad000[0xAF8];
    ComponentCollection<CEnvironmentSettings> m_pSettingsCollection;
    BYTE PadB08[0x38C];
    float m_TimeScale; // 0xE94
    float m_TimeOfDay; // 0xE98

  public:
    static CDynamicEnvironment* Singleton()
    {
      __int64 ptr1 = *(__int64*)((__int64)FCHandle + 0x4AD4558);
      return *(CDynamicEnvironment**)(ptr1 + 0x10);
    }
  };

  class CFireUiManagerImpl
  {
  public:
    BYTE Pad000[0x218];
    BYTE m_DisableUpdating;

  public:
    static CFireUiManagerImpl* Singleton()
    {
      __int64 pCUILayout = *(__int64*)((__int64)FCHandle + 0x4D96668);
      return *(CFireUiManagerImpl**)(pCUILayout + 0x30);
    }
  };

  class CEnvironmentExposure
  {
  public:
    BYTE Pad000[0x8];
    float m_ExposureMin; // Scene minimum exposure in EVs
    float m_ExposureMax; // Scene maximum exposure in EVs
    float m_ExposureRetarget; // EV retargeting curve from -20 to +20
  };

  class CEnvironmentSettings
  {
  public:
    BYTE Pad000[0x30];
    CEnvironmentExposure* m_pExposure;
    BYTE Pad038[0xD8];
  };

  class CMarketingCamera
  {
  public:
    virtual void Func1();
    __int64 m_pTransform;
    BYTE Pad010[0x4C];
    float m_FieldofView; // 0x5C
    BYTE Pad060[0x4];
    float m_NearPlane; // 0x64
    float m_FarPlane; // 0x68
    BYTE Pad06C[0x254];
    BYTE m_DofEnable; //0x2C0
    BYTE Pad2C1[0x17];
    BYTE m_DofOverride; // 0x2D8
    BYTE Pad2D9[0x3];
    float m_DofFocusDistance; // 0x2DC 
    float m_DofNear; // 0x2E0
    float m_DofFar; // 0x2E4
    float m_DofCoC; // 0x2E8
    BYTE Pad2EC[0xCC];
    float m_pitch; // 0x3B8
    float m_roll; // 0x3BC
    float m_yaw; // 0x3C0

  public:
    void OverwriteTransform(DirectX::XMFLOAT4X4 const& trans)
    {
      __int64 ptr1 = *(__int64*)(m_pTransform + 0x10);

      DirectX::XMFLOAT4X4* pCameraTrans = (DirectX::XMFLOAT4X4*)(ptr1 + 0x30);
      *pCameraTrans = trans;
    }

    DirectX::XMFLOAT4X4& GetTransform()
    {
      __int64 ptr1 = *(__int64*)(m_pTransform + 0x10);

      DirectX::XMFLOAT4X4* pCameraTrans = (DirectX::XMFLOAT4X4*)(ptr1 + 0x30);
      return *pCameraTrans;
    }
  };

  class CMarketingCameraActivator
  {
  public:
    virtual void Func1();
    virtual void Func2();
    virtual void Func3();
    virtual void HandleEvent(int* pEventID);

    BYTE Pad008[0x180];
    bool m_CameraEnabled;

  public:
    void ToggleCamera()
    {
      int phonyToggleEvent = 0x646DE935;
      HandleEvent(&phonyToggleEvent);
    }
  };

  class CRenderGeometryConfig
  {
  public:
    BYTE Pad000[0x7C];
    float m_ClustersLodScale; // 0x7C
    float m_GrassClustersLodScale; // 0x80
    int m_ClusterMultiplier; // 0x84
    float m_LodScale; // 0x88
    float m_KillLodScale; // 0x8C
    float m_ClusterZoomFactorScale; // 0x90
    float m_DetailDistanceScaleModifier; // 0x94

  public:
    static CRenderGeometryConfig* GetActiveConfig()
    {
      __int64 ptr1 = *(__int64*)((__int64)FCHandle + 0x4CC9150);
      return (CRenderGeometryConfig*)(ptr1 + 0x9D0);
    }
  };

  class CTimer
  {
  public:
    BYTE Pad000[0x90];
    BYTE m_FreezeTime;
    BYTE Pad091[0x7];
    double m_TimeScale;

  public:
    static CTimer* Singleton()
    {
      return *(CTimer**)((__int64)FCHandle + 0x4CC7778);
    }
  };

  static IDXGISwapChain* GetSwapChain()
  {
    __int64 ptr1 = *(__int64*)((__int64)FCHandle + 0x4CCAEE0);
    __int64 ptr2 = *(__int64*)(ptr1 + 0x1B0);

    return *(IDXGISwapChain**)(ptr2 + 0x8);
  }

  static ID3D11Device* GetDevice()
  {
    __int64 ptr1 = *(__int64*)((__int64)FCHandle + 0x4CCAEE0);

    return *(ID3D11Device**)(ptr1 + 0x40);
  }

  static bool IsFocused()
  {
    HWND focusedWindow = GetForegroundWindow();
    return FCHwnd == focusedWindow;
  }

  static bool IsMouseConfined()
  {
    __int64 pMouseThing = *(__int64*)((__int64)FCHandle + 0x4CC7818);
    return *(bool*)(pMouseThing + 0x30);
  }
}