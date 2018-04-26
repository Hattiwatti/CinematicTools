#pragma once
#include "Util/Util.h"
#include <d3d11.h>
#include <vector>

namespace Apex
{
  class CClock
  {
  public:
    BYTE Pad000[0x24];
    float m_TimeScale;

  public:
    static CClock* Singleton()
    {
      return *(CClock**)(util::offsets::GetOffset("OFFSET_CLOCK"));
    }
  };

  class CEnvironmentGFXManager
  {
    struct WeatherInfo
    {
      unsigned int* pIndex;
      int unk1;
      float unk2;
      float unk3;
      float unk4;
      int unk5;
      int unk6;
    };

  public:
    BYTE Pad000[0x10];
    std::vector<WeatherInfo> m_WeatherStack;
    BYTE Pad028[0x40];
    std::vector<__int64> m_WeatherPresets;
  };

  class CGraphicsEngine
  {
    struct DxStruct
    {
      BYTE Pad000[0x20];
      IDXGISwapChain* SwapChain;
      ID3D11Device* Device;
    };

  public:
    BYTE Pad000[0xE78];
    DxStruct* m_D3Objects;

  public:
    static CGraphicsEngine* Singleton()
    {
      return *(CGraphicsEngine**)(util::offsets::GetOffset("OFFSET_GRAPHICSENGINE"));
    }
  };

  class CUIManager
  {
  public:
    BYTE Pad000[0x16A];
    BYTE m_Enabled;

  public:
    static CUIManager* Singleton()
    {
      return *(CUIManager**)(util::offsets::GetOffset("OFFSET_UIMANAGER"));
    }
  };

  class CWorldTime
  {
  public:
    BYTE Pad000[0xE8];
    float m_TimeOfDay;
    BYTE Pad0EC[0x4];
    float m_TimeScale;

  public:
    static CWorldTime * Singleton()
    {
      return *(CWorldTime**)(util::offsets::GetOffset("OFFSET_WORLDTIME"));
    }
  };

  static float* GetTimeScale()
  {
    return (float*)(util::offsets::GetOffset("OFFSET_TIMESCALE"));
  }
}