#include "Util.h"
/*
#include <boost/assign.hpp>
#include <fstream>
#include <Psapi.h>

using namespace boost::assign;
using namespace util;

namespace
{
  std::vector<offsets::Signature*> m_signatures;
  std::map<std::string, __int64> m_vtables;

  std::map<std::string, __int64> m_hardcodedOffsets = map_list_of
  ("OFFSET_DXRENDERER", 0x1438068F8)
    ("OFFSET_GAMERENDERER", 0x1438067C8)
    ("OFFSET_WORLDRENDERER", 0x1437F7240)
    ("OFFSET_SYSTEM", 0x1437FAB10)

    ("OFFSET_GAMERENDERSETTINGS", 0x143982948)
    ("OFFSET_GAMETIMESETTINGS", 0x143982958)
    ("OFFSET_UISETTINGS", 0x143983170)
    ("OFFSET_MESHSETTINGS", 0x143983020)

    ("OFFSET_CLIENTGAMECONTEXT", 0x1437D2890)
    ("OFFSET_MAIN", 0x1439467D0)

    ("OFFSET_GENERICCAMERAUPDATE", 0x140FB5530)
    ("OFFSET_TIMEOFDAY1", 0x1412573E0)
    ("OFFSET_TIMEOFDAY2", 0x14183D2D0)

    ("OFFSET_DEPTHUPDATE", 0x141A27020)
    ("OFFSET_ONRESIZE", 0x141A94B40)
    ("OFFSET_FACTORYHOOK", 0x141B586A0)
    ("OFFSET_EXITHOOK", 0x1407E6B20)

    ("OFFSET_LIGHTMANAGER", 0x1437D30B0)
    ("OFFSET_FIRSTTYPEINFO", 0x1437B7130)
    ("OFFSET_RESOURCEMANAGER", 0x14359A818);

  std::map<std::string, __int64> m_hardcodedVtables = map_list_of
  ("PBRSpotLightEntity", 0x1428D2508)
    ("PbrSpotLightEntityData", 0x142BF1960) //
    ("PBRSphereLightEntity", 0x1428D2430)
    ("PbrSphereLightEntityData", 0x1428D70C0);

  HMODULE m_handle;

  bool DataCompare(BYTE* pData, BYTE* bSig, char* szMask)
  {
    for (; *szMask; ++szMask, ++pData, ++bSig)
    {
      if (*szMask == 'x' && *pData != *bSig)
        return false;
    }
    return (*szMask) == 0;
  }

  BYTE* FindPattern(BYTE* dwAddress, __int64 dwSize, BYTE* pbSig, char* szMask)
  {
    register BYTE bFirstByte = *(BYTE*)pbSig;

    __int64 length = (__int64)dwAddress + dwSize - strlen(szMask);

    for (register __int64 i = (__int64)dwAddress; i < length; i += 4) // might run faster with 8 bytes but I am too lazy
    {
      unsigned x = *(unsigned*)(i);

      if ((x & 0xFF) == bFirstByte)
        if (DataCompare(reinterpret_cast<BYTE*>(i), pbSig, szMask))
          return reinterpret_cast<BYTE*>(i);

      if ((x & 0xFF00) >> 8 == bFirstByte)
        if (DataCompare(reinterpret_cast<BYTE*>(i + 1), pbSig, szMask))
          return reinterpret_cast<BYTE*>(i + 1);

      if ((x & 0xFF0000) >> 16 == bFirstByte)
        if (DataCompare(reinterpret_cast<BYTE*>(i + 2), pbSig, szMask))
          return reinterpret_cast<BYTE*>(i + 2);

      if ((x & 0xFF000000) >> 24 == bFirstByte)
        if (DataCompare(reinterpret_cast<BYTE*>(i + 3), pbSig, szMask))
          return reinterpret_cast<BYTE*>(i + 3);
    }
    return 0;
  }
}

offsets::Signature::Signature(std::string name, std::string pattern, int offset = 0)
{
  m_offset = offset;
  m_isFunction = true;
  m_name = name;

  int Index = 0;
  const char* pChar = &pattern.c_str()[0];

  m_pattern = new BYTE[pattern.size()];
  m_mask = new CHAR[pattern.size()];

  ZeroMemory(m_pattern, pattern.size());
  ZeroMemory(m_mask, pattern.size());

  while (*pChar)
  {
    if (*pChar == ' ')
    {
      pChar++;
      continue;
    }
    else if (*pChar == ']')
    {
      pChar++;
      m_size = Index - m_offset;
      continue;
    }
    else if (*pChar == '[')
    {
      this->m_isFunction = false;
      pChar++;
      m_offset = Index;
      continue;
    }
    else if (*pChar == '?')
    {
      m_mask[Index++] += '?';
      pChar += 2;
      continue;
    }

    m_mask[Index] = 'x';
    m_pattern[Index++] = (util::CharToByte(pChar[0]) << 4) + util::CharToByte(pChar[1]);
    pChar += 2;
  }
}

void offsets::Init()
{
  m_signatures.push_back(new Signature("OFFSET_DXRENDERER", "48 8B 05 [ ?? ?? ?? ?? ] 48 85 C0 74 09 48 8B 80")); //
  m_signatures.push_back(new Signature("OFFSET_GAMERENDERER", "48 89 1D [ ?? ?? ?? ?? ] 48 8B 03 48 8D 0D"));
  m_signatures.push_back(new Signature("OFFSET_WORLDRENDERER", "48 89 35 [ ?? ?? ?? ?? ] BA ?? ?? ?? ?? 45 8D 46 10 49 8B CD"));
  m_signatures.push_back(new Signature("OFFSET_SYSTEM", "0F 84 ?? ?? ?? ?? 48 8B 3D [ ?? ?? ?? ?? ] BA"));

  m_signatures.push_back(new Signature("OFFSET_GAMERENDERSETTINGS", "48 8B 05 [ ?? ?? ?? ?? ] 48 3B DF"));
  m_signatures.push_back(new Signature("OFFSET_GAMETIMESETTINGS", "48 89 05 [ ?? ?? ?? ?? ] C7 40 ?? ?? ?? ?? ?? 8B 43 18"));
  m_signatures.push_back(new Signature("OFFSET_UISETTINGS", "48 8B 0D [ ?? ?? ?? ?? ] 48 8B 51 28 48 8B C8"));
  m_signatures.push_back(new Signature("OFFSET_MESHSETTINGS", "E8 ?? ?? ?? ?? 48 89 05 [ ?? ?? ?? ?? ] F3 0F 10 40 ?? F3 0F 11 45"));

  m_signatures.push_back(new Signature("OFFSET_CLIENTGAMECONTEXT", "48 89 35 [ ?? ?? ?? ?? ] 33 D2"));
  m_signatures.push_back(new Signature("OFFSET_MAIN", "48 3B 1D [ ?? ?? ?? ?? ] 48 0F 47 F7 48 85 F6 74 22"));

  m_signatures.push_back(new Signature("OFFSET_GENERICCAMERAUPDATE", "49 8D 97 ?? ?? ?? ?? 49 8B CF E8 [ ?? ?? ?? ?? ] 48 8B 5C 24"));

  m_signatures.push_back(new Signature("OFFSET_TIMEOFDAY1", "E8 [ ?? ?? ?? ?? ] 48 8B 8B ?? ?? ?? ?? F3 0F 11 83"));
  m_signatures.push_back(new Signature("OFFSET_TIMEOFDAY2", "E8 [ ?? ?? ?? ?? ] F3 41 0F 11 47"));
  m_signatures.push_back(new Signature("OFFSET_DEPTHUPDATE", "48 89 7C 24 ?? 48 89 5C 24 ?? E8 [ ?? ?? ?? ?? ] 48 8B 06"));
  m_signatures.push_back(new Signature("OFFSET_ONRESIZE", "4D 8B 41 10 49 8B 51 08 49 8B 09 E8 [ ?? ?? ?? ?? ]"));
  m_signatures.push_back(new Signature("OFFSET_FACTORYHOOK", "48 8B D0 49 8B CD E8 [ ?? ?? ?? ?? ] 0F 57 C0"));
  m_signatures.push_back(new Signature("OFFSET_EXITHOOK", "E8 [ ?? ?? ?? ?? ] 48 8B 8F ?? ?? ?? ?? 48 85 C9 74 1B"));

  m_signatures.push_back(new Signature("OFFSET_LIGHTMANAGER", "45 84 FF 74 15 48 8B 0D [ ?? ?? ?? ?? ]"));
  m_signatures.push_back(new Signature("OFFSET_FIRSTTYPEINFO", "48 8B 05 [ ?? ?? ?? ?? ] 48 89 41 08 48 89 0D"));
  m_signatures.push_back(new Signature("OFFSET_RESOURCEMANAGER", "45 8B CE 48 8B 15 [ ?? ?? ?? ?? ]"));

  m_handle = GetModuleHandleA("NeedForSpeedPayback.exe");
  if (!m_handle) m_handle = GetModuleHandleA("NeedForSpeedPaybackTrial.exe");

  if (!m_handle)
  {
    log::Error("Could not get a handle for the executable");
    return;
  }

  log::Write("Scanning for offsets...");

  MODULEINFO info;
  GetModuleInformation(GetCurrentProcess(), m_handle, &info, sizeof(MODULEINFO));

  for (auto itr = m_signatures.begin(); itr != m_signatures.end(); ++itr)
  {
    Signature* sig = *itr;
    BYTE* result = FindPattern((BYTE*)info.lpBaseOfDll, (__int64)info.SizeOfImage, sig->m_pattern, sig->m_mask);
    //log::Write("%s result 0x%I64X", sig->m_name.c_str(), result);
    if (result)
    {
      if (sig->m_isFunction)
      {
        sig->m_result = ((__int64)result + (__int64)sig->m_offset);
      }
      else
      {
        int* relative = (int*)((__int64)result + (__int64)sig->m_offset);
        sig->m_result = ((__int64)result + (__int64)sig->m_offset + 0x4 + (__int64)*relative);
      }
    }
  }

  bool allFound = true;
  std::fstream file;
  file.open("./Cinematic Tools/Offsets.log", std::fstream::in | std::fstream::out | std::fstream::trunc);

  fb::BuildInfo* pBuildInfo = fb::BuildInfo::Singleton();
  file << pBuildInfo->getBranchName() << " - " << pBuildInfo->getLicenseeId() << " - " << pBuildInfo->getBuildDate() << std::endl;
  for (std::vector<Signature*>::iterator itr = m_signatures.begin(); itr != m_signatures.end(); ++itr)
  {
    file << "(\"" + (*itr)->m_name + "\", \t\t0x" << std::hex << std::uppercase << (*itr)->m_result << " )\n";
    if (!(*itr)->m_result)
    {
      log::Error("Could not find %s", (*itr)->m_name.c_str());
      allFound = false;
    }
  }

  if (allFound) log::Ok("All offsets found");

  //file << "\n";
  //file << "(\"PBRSpotLightEntity\", \t\t0x" << std::hex << std::uppercase << FindVTable("PBRSpotLightEntity") << " )\n";
  //file << "(\"PbrSpotLightEntityData\", \t\t0x" << std::hex << std::uppercase << FindVTable("PbrSpotLightEntityData") << " )\n";
  //file << "(\"PBRSphereLightEntity\", \t\t0x" << std::hex << std::uppercase << FindVTable("PBRSphereLightEntity") << " )\n";
  //file << "(\"PbrSphereLightEntityData\", \t\t0x" << std::hex << std::uppercase << FindVTable("PbrSphereLightEntityData") << " )\n";

  file.close();
  printf("\n");
}
__int64 offsets::FindVTable(std::string name)
{
  MODULEINFO info;
  if (!m_handle)
  {
    m_handle = GetModuleHandleA("bf1.exe");
    if (!m_handle) m_handle = GetModuleHandleA("bf1Trial.exe");
    if (!m_handle) m_handle = GetModuleHandleA("bf1_cte.exe");
  }

  GetModuleInformation(GetCurrentProcess(), m_handle, &info, sizeof(MODULEINFO));

  fb::TypeInfo* pTypeInfo = *(fb::TypeInfo**)GetOffset("OFFSET_FIRSTTYPEINFO");
  while (pTypeInfo != nullptr)
  {
    if (std::string(pTypeInfo->m_MemberInfo->m_Name) == name)
      break;

    pTypeInfo = pTypeInfo->m_NextTypeInfo;
  }

  if (pTypeInfo == nullptr)
  {
    log::Error("Couldn't find typeinfo for %s", name.c_str());
    return 0;
  }

  BYTE* start = (BYTE*)info.lpBaseOfDll;
  __int64 size = (__int64)info.SizeOfImage;

  char* asmSignature = "\x48\x8D\x05\x00\x00\x00\x00\xC3";
  char* asmMask = "xxx????x";

  BYTE* result = 0;
  bool found = false;

  while ((result = FindPattern(start, size, (PBYTE)asmSignature, asmMask)) != 0)
  {
    int* relative = (int*)((INT64)result + (INT64)0x3);
    INT64 address = ((INT64)result + 0x7 + *relative);
    if ((__int64)address == (__int64)pTypeInfo)
    {
      found = true;
      break;
    }

    size = size - (((__int64)result + 0x7) - (__int64)start);
    start = (BYTE*)((__int64)result + 0x7);
  }

  if (!found)
  {
    log::Error("Couldn't find vtable for %s", name.c_str());
    return 0;
  }

  char* mask = "xxxxxxxx";
  result = FindPattern((BYTE*)info.lpBaseOfDll, (__int64)info.SizeOfImage, (PBYTE)&result, mask);
  m_vtables.insert(std::pair<std::string, __int64>(name, (__int64)result));

  //log::Write("%s 0x%I64X", name.c_str(), result);
  return (__int64)result;
}

__int64 offsets::GetOffset(std::string name)
{
  for (auto itr = m_signatures.begin(); itr != m_signatures.end(); ++itr)
  {
    if ((*itr)->m_name == name)
    {
      if (!(*itr)->m_result)
        break;
      return (*itr)->m_result;
    }
  }

  auto result = m_hardcodedOffsets.find(name);
  if (result != m_hardcodedOffsets.end())
  {
    return result->second;
  }

  log::Error("Couldn't find %s", name.c_str());
  return NULL;
}

__int64 offsets::GetVTable(std::string name)
{
  auto result = m_vtables.find(name);
  if (result != m_vtables.end())
  {
    return result->second;
  }

  result = m_hardcodedVtables.find(name);
  if (result != m_hardcodedVtables.end())
  {
    return result->second;
  }

  log::Error("Couldn't find %s", name.c_str());
  return NULL;
}

*/