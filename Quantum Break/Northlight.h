#pragma once
#include <d3d11.h>
#include <Windows.h>

extern HMODULE g_aiModule;
extern HMODULE g_d3dModule;

namespace Northlight
{
  namespace ai
  {
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
  }
}