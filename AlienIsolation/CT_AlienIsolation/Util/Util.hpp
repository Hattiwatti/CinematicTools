#pragma once
#include <Windows.h>
#include "../FB SDK/Frostbite.h"

namespace Util
{
  inline BOOL WriteMemory(DWORD_PTR dwAddress, const void* cpvPatch, DWORD dwSize)
  {
    DWORD dwProtect;
    if (VirtualProtect((void*)dwAddress, dwSize, PAGE_READWRITE, &dwProtect)) //Unprotect the memory
      memcpy((void*)dwAddress, cpvPatch, dwSize); //Write our patch
    else

      return false; //Failed to unprotect, so return false..
    return VirtualProtect((void*)dwAddress, dwSize, dwProtect, new DWORD); //Reprotect the memory
  }

  inline std::string GetMapName()
  {
    fb::LevelData* pData = fb::ClientGameContext::Singleton()->m_level->m_levelData;
    std::string assetName(pData->m_name.GetString());

    size_t pos = assetName.find_last_of('/');
    if (pos == std::string::npos) return std::string(pData->m_description.m_name.GetString());

    assetName = assetName.substr(pos + 1);
    return assetName;
  }
}