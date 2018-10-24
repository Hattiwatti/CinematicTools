#pragma once
#include <d3d11.h>
#include <Windows.h>

extern HMODULE g_aiModule;
extern HMODULE g_d3dModule;
extern HMODULE g_rendererModule;
extern HMODULE g_rlModule;
extern HMODULE g_qbModule;

namespace Northlight
{
  namespace ai
  {
    class Character;
    class WorldConception;

    class AIManager
    {
    public:
      BYTE Pad000[0x30];
      int m_CharacterCount;

    public:
      static AIManager* Singleton()
      {
        typedef AIManager*(__stdcall* tGetInstance)();

        tGetInstance AIManagerGetInstance = (tGetInstance)GetProcAddress(g_aiModule, "?getInstance@AIManager@ai@@SAPEAV12@XZ");
        return AIManagerGetInstance();
      }

      static int GetCharacters(Character** pCharacters)
      {
        AIManager* pAIManager = Singleton();
        for (int i = 0; i <= pAIManager->m_CharacterCount; ++i)
        {
          Character** ppCharacter = (Character**)((__int64)pAIManager + 0x38 + i * 8);
          pCharacters[i] = *ppCharacter;
        }

        return pAIManager->m_CharacterCount + 1;
      }
    };

    class Character
    {
    public:
      virtual void Func1();
      virtual void init();
      virtual void update(float);
      virtual void start();
      virtual void Func5();
      virtual void onActivated();
      virtual void Func7();
      virtual void Func8();
      virtual void postLoad();
      virtual const wchar_t* getProfileNameW();
      virtual const char* getProfileName();
      virtual void toString(__int64 r_string);
      virtual void legacySerialize(__int64 aiSaver);
      virtual void Func14();

      BYTE Pad008[0x40];
      WorldConception* m_pWorldConception;

    public:
      void SetSuspended(bool val)
      {
        typedef void(__fastcall* tCharacterSetSuspended)(Character*, bool);

        tCharacterSetSuspended CharacterSetSuspended = (tCharacterSetSuspended)GetProcAddress(g_aiModule, "?setSuspended@Character@ai@@QEAAX_N@Z");
        CharacterSetSuspended(this, val);
      }
    };

    class WorldConception
    {
    public:
      BYTE Pad000[0xA10];
      float m_position[3];
    };
  }

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

        tGetDevice oGetDevice = (tGetDevice)GetProcAddress(g_d3dModule, "?getDevice@Device@d3d@@QEAAPEAXXZ");
        return oGetDevice();
      }

      static ID3D11DeviceContext* GetContext()
      {
        typedef ID3D11DeviceContext*(__fastcall* tGetContext)();

        tGetContext oGetContext = (tGetContext)GetProcAddress(g_d3dModule, "?getDeviceContext@Device@d3d@@QEAAPEAXXZ");
        return oGetContext();
      }

      static Device* Singleton()
      {
        typedef Device*(__fastcall* tGetInstance)();

        tGetInstance GetInstance = (tGetInstance)GetProcAddress(g_d3dModule, "?getInstanceUnchecked@Device@d3d@@SAPEAV12@XZ");
        return GetInstance();
      }
    };
  }

  namespace r
  {
    class ClassTypeInfo
    {
    public:
      virtual const char* GetTypeName();
      virtual int GetCRCSum();
      virtual int GetClassId();
      virtual ClassTypeInfo* GetBaseClass();
      virtual void Func5();
      virtual void Func6();
      virtual void Func7();
      virtual __int64 Create();

      ClassTypeInfo* m_current;
      ClassTypeInfo* m_next;
    };

    class Time
    {
    public:
      BYTE Pad000[0x30];
      float m_Framerate;
      BYTE Pad034[0x4];
      float m_WorldTimeScale;
      float m_EffectTimeScale;
      float m_UnkTimeScale1;
      float m_UnkTimeScale2;
      BYTE Pad048[0x4];
      float m_VariableDelta;

    public:
      static Time* Singleton()
      {
        return *(Time**)(GetProcAddress(g_rlModule, "?sm_pInstance@Time@r@@0PEAV12@EA"));
      }
    };

    static bool IsPaused()
    {
      return *(bool*)((__int64)g_gameHandle + 0x116F5B2);
    }
  }

  namespace rend
  {

    class Spotlight
    {
    public:
      BYTE Pad000[0xAC0];

    public:
      Spotlight()
      {
        typedef __int64(__fastcall* tCreateSpotLight)(rend::Spotlight* pThis);
        tCreateSpotLight CreateSpotLight = (tCreateSpotLight)GetProcAddress(g_rendererModule, "??0SpotLight@rend@@QEAA@XZ");

        CreateSpotLight(this);
      }


    };

    static void UpdateResolution()
    {
      RECT windowRect;
      if (!GetWindowRect(g_gameHwnd, &windowRect))
        return;

      int width = windowRect.right - windowRect.left;
      int height = windowRect.bottom - windowRect.top;

      util::log::Write("UpdateResolution %d %d", width, height);
      if (width < 100 || height < 100)
        return;

      int* pResolution = (int*)((__int64)g_rendererModule + 0x49AE28);
      for (int i = 0; i < 2; ++i)
      {
        pResolution[i * 2] = width;
        pResolution[i * 2 + 1] = height;
      }
    }

    static void SetResolution(int width, int height)
    {
      int* pResolution = (int*)((__int64)g_rendererModule + 0x49AE28);

      for (int i = 0; i < 5; ++i)
      {
        pResolution[i * 2] = width;
        pResolution[i * 2 + 1] = height;
      }
    }
  
    static void SetDoFParameters(float bokeh, float fstop, float focalDistance)
    {
      typedef void(__fastcall* tSetDofParameter)(__int64, float);

      __int64 pDOFWrapper = *(__int64*)((__int64)g_rendererModule + 0x495628);
      tSetDofParameter SetBokehScale = (tSetDofParameter)((__int64)g_rendererModule + 0x819C0);
      SetBokehScale(pDOFWrapper, bokeh);
    }
  }

  static float* GetFrameRate()
  {
    return (float*)((__int64)g_qbModule + 0xA95C88);
  }

  static void SetGameFreezed(bool a1)
  {
    typedef void(__fastcall* tFreezeFunc)(bool);
    tFreezeFunc FreezeFunc = (tFreezeFunc)((__int64)g_gameHandle + 0x357610);
    FreezeFunc(a1);

    // xor edx, edx -> inc edx
    // So rend::VectorBlurWrapper::setFreeCameraMoved(bool) gets called with true
    // Fixes vector blur on zooming etc.
    if (a1)
      util::WriteMemory((__int64)g_gameHandle + 0x636E7F, "\xFF\xC2", 2);
    else
      util::WriteMemory((__int64)g_gameHandle + 0x636E7F, "\x33\xD2", 2);
  }
}