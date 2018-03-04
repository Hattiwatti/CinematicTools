#pragma once
#include <Windows.h>
#include "Character.h"

namespace ai
{
  class AIManager
  {
  public:
    BYTE Pad000[0x30];
    int m_CharacterCount;

  public:
    static AIManager* Singleton()
    {
      typedef AIManager*(__stdcall* tGetInstance)();

      HMODULE hModule = GetModuleHandleA("ai_x64_f.dll");
      tGetInstance AIManagerGetInstance = (tGetInstance)GetProcAddress(hModule, "?getInstance@AIManager@ai@@SAPEAV12@XZ");
      
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
}