#pragma once
#include "WorldConception.h"

namespace ai
{
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
      typedef void(__fastcall* tChracterSetSuspended)(Character*, bool);

      HMODULE hModule = GetModuleHandleA("ai_x64_f.dll");
      tChracterSetSuspended CharacterSetSuspended = (tChracterSetSuspended)GetProcAddress(hModule, "?setSuspended@Character@ai@@QEAAX_N@Z");

      CharacterSetSuspended(this, val);
    }
  };
}