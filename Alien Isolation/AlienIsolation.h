#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "Util/Util.h"

using namespace DirectX;

namespace CATHODE
{
  struct AICameraState
  {
    XMFLOAT4 m_Rotation;
    XMFLOAT3 m_Position;
    float m_FieldOfView;
    float m_NearPlane;
    float m_FarPlane;
  };

  class AICamera
  {
  public:
    BYTE Pad000[0x4];
    AICameraState m_State[3];
  };

  class AICameraManager
  {
  public:
    BYTE Pad000[0x1D8];
    AICamera* m_ActiveCamera;
  };

  class Character
  {
  public:
    BYTE Pad000[0x10];
    char m_Name[0x10]; // 0x10
    BYTE Pad020[0x30];
    unsigned int m_ID; // 0x50
    BYTE Pad054[0x20C];
    bool m_Active; // 0x260
    BYTE Pad261[0x78];
    bool m_Hidden;  // 0x2D9
    bool m_Hidden2; // 0x2DA
    BYTE Pad2DB[0x125];
    XMFLOAT4X4 m_Transform; // 0x400
    BYTE Pad440[0xB0C];
    bool m_Animate; // 0xF4C
  };

  class CharacterManager
  {
  public:
    BYTE Pad000[0x80];
    Character** m_PlayerCharacters; // 0x80
    unsigned int m_PlayerCharacterCount; // 0x84
    BYTE Pad088[0x48];
    Character** m_NPCCharacters; // 0xD0
    unsigned int m_NPCCharacterCount; // 0xD4
  };

  class D3D
  {
  public:
    BYTE Pad000[0x4];
    ID3D11Device* m_pDevice;
    IDXGISwapChain* m_pSwapChain;

  public:
    static D3D* Singleton()
    {
      return *(D3D**)(util::offsets::GetOffset("OFFSET_D3D"));
    }
  };

  class Main
  {
  public:
    BYTE Pad000[0x8];
    CharacterManager* m_CharacterManager; // 0x8
    BYTE Pad00C[0x4C];
    AICameraManager* m_CameraManager; // 0x58

  public:
    static Main* Singleton()
    {
      return *(Main**)(util::offsets::GetOffset("OFFSET_MAIN"));
    }
  };

  struct PostProcessSettings
  {
    uint16_t m_Priority;
    uint16_t m_BlendMode;
    float m_Intensity;
  };

  class BlendLowResFrame
  {
  public:
    PostProcessSettings m_PostProcessSettings;
    float m_BlendValue;
  };

  class BloomSettings
  {
  public:
    PostProcessSettings m_PostProcessSettings;
    float m_FrameBufferScale;
    float m_FrameBufferOffset;
    float m_BloomScale;
    float m_BloomGatherExponent;
    float m_BloomGatherScale;
  };

  class ChromaticAberrations
  {
  public:
    PostProcessSettings m_PostProcessSettings;
    float m_AberrationScalar;
  };

  class ColorSettings
  {
  public:
    PostProcessSettings m_PostProcessSettings;
    float m_Brightness;
    float m_Contrast;
    float m_Saturation;
    XMFLOAT3 m_TintColor;
  };

  class DayToneMapSettings
  {
  public:
    BYTE Pad000[0xC];
    float m_BlackPoint;
    float m_CrossOverPoint;
    float m_WhitePoint;
    float m_ShoulderStrength;
    float m_ToeStrength;
    float m_LuminanceScale;
  };

  class DistortionSettings
  {
  public:
    PostProcessSettings m_PostProcessSettings;
    float m_RadialDistortFactor;
    float m_RadialDistortConstraint;
    float m_RadialDistortScalar;
  };

  class DepthOfFieldSettings
  {
  public:
    PostProcessSettings m_PostProcessSettings;
    float m_FocalLength;
    float m_FocalPlane;
    float m_FNum;
  };

  class FilmGrainSettings
  {
  public:
    PostProcessSettings m_PostProcessSettings;
    float m_LowLumAmplifier;
    float m_MidLumAmplifier;
    float m_HighLumAmplifier;
    float m_LowLumRange;
    float m_MidLumRange;
    float m_HighLumRange;
    float m_AdaptationScalar;
    float m_AdaptationTimeScalar;
    float m_UnadaptedLowLumAmplifier;
    float m_UnadaptedMidLumAmplifier;
    float m_UnadaptedHighLumAmplifier;
    float m_NoiseTextureScale;
  };

  class FlareSettings
  {
  public:
    PostProcessSettings m_PostProcessSettings;
    float m_FlareOffset0;
    float m_FlareIntensity0;
    float m_FlareAttenuation0;
    float m_FlareOffset1;
    float m_FlareIntensity1;
    float m_FlareAttenuation1;
    float m_FlareOffset2;
    float m_FlareIntensity2;
    float m_FlareAttenuation2;
    float m_FlareOffset3;
    float m_FlareIntensity3;
    float m_FlareAttenuation3;
  };

  class FullScreenBlurSettings
  {
  public:
    PostProcessSettings m_PostProcessSettings;
    float m_Contribution;
  };

  class FullScreenOverlay
  {
  public:
    PostProcessSettings m_PostProcessSettings;
    float m_ThresholdValue;
    float m_ThresholdStart;
    float m_ThresholdStop;
    float m_ThresholdRange;
    float m_AlphaScalar;
  };

  class HighSpecMotionBlurSettings
  {
  public:
    PostProcessSettings m_PostProcessSettings;
    float m_Contribution;
    float m_CameraVelocityScalar;
    float m_CameraVelocityMin;
    float m_CameraVelocityMax;
    float m_ObjectVelocityScalar;
    float m_ObjectVelocityMin;
    float m_ObjectVelocityMax;
    float m_BlurRange;
  };

  class LensDustSettings
  {
  public:
    PostProcessSettings m_PostProcessSettings;
    float m_MaxReflectedBloomIntensity;
    float m_ReflectedBloomIntensityScalar;
    float m_MaxBloomIntensity;
    float m_BloomIntensityScalar;
    float m_Threshold;
  };

  class SharpnessSettings
  {
  public:
    PostProcessSettings m_PostProcessSettings;
    float m_LocalContrastFactor;
  };

  class VignetteSettings
  {
    PostProcessSettings m_PostProcessSettings;
    float m_VignetteFactor;
    float m_VignetteChromaticAberrationScale;
  };

  class PostProcessNew
  {
  public:
    ColorSettings m_ColorSettings;
    BloomSettings m_BloomSettings; // 0x1A90
    FlareSettings m_FlareSettings; // 0x1A9C
    HighSpecMotionBlurSettings m_HighSpecMotionBlurSettings; // 0x1AA8
    FilmGrainSettings m_FilmGrainSettings; // 0x1AB4
    VignetteSettings m_VignetteSettings; // 0x1AC0
    DistortionSettings m_DistortionSettings; // 0x1ACC
    SharpnessSettings m_SharpnessSettings; // 0x1AD8
    LensDustSettings m_LensDustSettings; // 0x1AE4
    FullScreenBlurSettings m_FullScreenBlurSettings; // 0x1AF0
    DepthOfFieldSettings m_DepthOfFieldSettings; // 0x1AFC
    FullScreenOverlay m_FullScreenOverlaySettings; // 0x1B08
    BlendLowResFrame m_BlendLowResFrame; // 0x1B14
    ChromaticAberrations m_ChromaticAberrations; // 0x1B20
  };

  class PostProcess
  {
  public:
    float m_Contrast; // 0x0
    float m_Saturation; // 0x4
    XMFLOAT3 m_TintColor; // 0x8
    BYTE Pad014[0x8]; // 0x14
    float m_Brightness; // 0x1C
    float m_Gamma; // 0x20
    BYTE Pad024[0x8]; // 0x24
    float m_BloomScale; // 0x2C
    BYTE Pad030[0x18]; // 0x30
    float m_lenseThing; // 0x48
    BYTE Pad04C[0x4C]; // 0x4C
    float m_FilmGrainScale1; // 0x98
    float m_FilmGrainScale2; // 0x9C
    float m_FilmGrainLightScale; // 0xA0
    BYTE Pad2[0x2C]; // 0xA4
    float m_VignetteStrength; // 0xD0
    float m_ChromaticStrength; // 0xD4
    BYTE Pad3[0x8]; // 0xD8
    float m_RadialDistortFactor; // 0xE0
    float m_RadialDistortConstraint; // 0xE4
    float m_RadialDistortScalar; // 0xE8
    BYTE Pad4[0x30]; // 0xEC
    float m_Sharpness; // 0x11C
    BYTE Pad5[0x8]; // 0x120
    float m_DofStrength; // 0x128
    float m_DofFocusDistance; // 0x12C
    float m_DofScale; // 0x130
  };

  class PostProcessSystem
  {
  public:
    BYTE Pad000[0x380];
    DayToneMapSettings* m_TonemapSettings;

  public:
    static PostProcessSystem* Singleton()
    {
      return (PostProcessSystem*)(util::offsets::GetOffset("OFFSET_POSTPROCESS"));
    }
  };

  class ScaleformObject
  {
  public:
    BYTE Pad000[0x28];
    bool m_Active; // 0x28
    bool m_ForceHide; // 0x29
    BYTE Pad02A[0xA];
    const char* m_Name; // 0x34
  };

  class Scaleform
  {
  public:
    BYTE Pad000[0x38];
    ScaleformObject** m_ScaleformObjects;
    unsigned int m_ObjectCount;

  public:
    static Scaleform* Singleton()
    {
      return *(Scaleform**)(util::offsets::GetOffset("OFFSET_SCALEFORM"));
    }
  };

  static XMFLOAT4X4* GetCameraMatrix()
  {
    typedef XMFLOAT4X4*(__thiscall* tGetMatrix)(int _this);
    tGetMatrix GetMatrix = (tGetMatrix)(util::offsets::GetOffset("OFFSET_GETCAMERAMATRIX"));

    int unkPointer = *(int*)((int)GetModuleHandleA("AI.exe") + 0x1366A00);
    return GetMatrix(unkPointer);
  }

  static void ShowMouse(bool val)
  {
    int ptr1 = *(int*)(util::offsets::GetOffset("OFFSET_SHOWMOUSE"));
    int ptr2 = *(int*)(ptr1 + 0x4A70);

    bool* pShowMouse = (bool*)(ptr2 + 0x10);
    *pShowMouse = !val;
  }

  static Character* GetPlayerChr()
  {
    CharacterManager* pChrMgr = Main::Singleton()->m_CharacterManager;

    if (pChrMgr->m_PlayerCharacterCount == 0)
      return nullptr;

    return pChrMgr->m_PlayerCharacters[0];
  }
}