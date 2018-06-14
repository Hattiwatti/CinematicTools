#include "Util.h"
#include "../Main.h"

#include <boost/assign.hpp>
#include <fstream>
#include <unordered_map>
#include <Psapi.h>

using namespace boost::assign;

namespace
{
  bool m_UseScannedResults = false;
  std::unordered_map<std::string, util::offsets::Signature> m_Signatures;

  // Fill with hardcoded offsets if you don't want to use scanning
  // These should be relative to the module base.
  std::unordered_map<std::string, __int64> m_HardcodedOffsets = map_list_of
  ("OFFSET_CLOCK", 0x1E32668)
  ("OFFSET_ENVIRONMENTGFX", 0x1E32660)
  ("OFFSET_GRAPHICSENGINE", 0x1DA7DE0)
  ("OFFSET_UIMANAGER", 0x1DA7DF0)
  ("OFFSET_WORLDTIME", 0x1E58720)
  ("OFFSET_TIMESCALE", 0x1CC4654)
    
  ("OFFSET_CAMERAUPDATE", 0x3016E0)
  ("OFFSET_CAMERAUPDATE2", 0x2F7E70)
  ("OFFSET_INPUTUPDATE", 0x3703D0);

  bool DataCompare(BYTE* pData, BYTE* bSig, const char* szMask)
  {
    for (; *szMask; ++szMask, ++pData, ++bSig)
    {
      if (*szMask == 'x' && *pData != *bSig)
        return false;
    }
    return (*szMask) == 0;
  }

  BYTE* FindPattern(BYTE* dwAddress, __int64 dwSize, BYTE* pbSig, const char* szMask)
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

util::offsets::Signature::Signature(std::string const& sig, int offset /* = 0 */)
{
  Pattern = new BYTE[sig.size()]();
  AddOffset = offset;

  unsigned int patternOffset = 0;
  for (unsigned int i = 0; i < sig.size(); ++i)
  {
    switch (sig[i])
    {
      case ' ':
      {
        break;
      }
      case '[':
      {
        HasReference = true;
        ReferenceOffset = patternOffset;
        break;
      }
      case ']':
      {
        ReferenceSize = patternOffset - ReferenceOffset;
        break;
      }
      case '?':
      {
        Mask += '?';
        // In signature it's clearer to mark one wildcard byte as ??
        // so skip the next character.
        patternOffset += 1;
        i += 1;
        break;
      }
      default:
      {
        Mask += 'x';
        // Process 2 characters into a single byte
        Pattern[patternOffset] = (util::CharToByte(sig[i]) << 4) + util::CharToByte(sig[i+1]);
        patternOffset += 1;
        i += 1;
      }
    }
  }
}

bool util::offsets::CheckVersion(const char* supportedVersion)
{
  Signature versionSignature("41 B8 ?? ?? ?? ?? 48 8D 15 [ ?? ?? ?? ?? ] 48 81 C1");
  std::string sVersion = "Version could not be retrieved";

  MODULEINFO info;
  GetModuleInformation(GetCurrentProcess(), g_gameHandle, &info, sizeof(MODULEINFO));

  __int64 result = (__int64)FindPattern((BYTE*)info.lpBaseOfDll, info.SizeOfImage, versionSignature.Pattern, versionSignature.Mask.c_str());
  if (result)
  {
    int* pReference = (int*)(result + versionSignature.ReferenceOffset);
    // Assembly reference is relative to the address after the reference
    versionSignature.Result = ((__int64)pReference + versionSignature.ReferenceSize) + *pReference;

    char* version = (char*)versionSignature.Result;
    sVersion = std::string(version);
  }

  if (sVersion == supportedVersion)
  {
    util::log::Ok("Current Code Version: %s", sVersion.c_str());
    util::log::Ok("Supported Code Version: %s", supportedVersion);
    util::log::Ok("Game versions match. Continuing..\n");
    return true;
  }
  else
  {
    util::log::Warning("!! VERSION MISMATCH !!");
    util::log::Warning("Current Code Version: %s", sVersion.c_str());
    util::log::Warning("Supported Code Version: %s", supportedVersion);
    util::log::Warning("Some features may not work until next tool update.");
    return false;
  }
}

void util::offsets::Scan()
{
  // Signature example
  // Scan memory and find this pattern. Question marks are wildcard bytes.
  // Brackets mean that the offset is extracted from the assembly reference
  // For example:
  // mov rax,[123456]
  // We want 123456, its place should be marked with wildcard bytes surrounded
  // by brackets in the signature.
  //
  // The last argument is the offset to be added to the result, useful when
  // you need a code offset for byte patches.

  m_Signatures.emplace("OFFSET_CLOCK", Signature("72 BB 48 8B 0D [ ?? ?? ?? ?? ]"));
  m_Signatures.emplace("OFFSET_ENVIRONMENTGFX", Signature("48 89 73 60 48 8B 0D [ ?? ?? ?? ?? ]"));
  m_Signatures.emplace("OFFSET_GRAPHICSENGINE", Signature("EB 36 48 8B 0D [ ?? ?? ?? ?? ]"));
  m_Signatures.emplace("OFFSET_UIMANAGER", Signature("48 8B 0D [ ?? ?? ?? ?? ] E8 ?? ?? ?? ?? 84 C0 74 0C 48 8B 0D"));
  m_Signatures.emplace("OFFSET_WORLDTIME", Signature("74 21 48 8B 05 [ ?? ?? ?? ?? ] 48 85 C0"));
  m_Signatures.emplace("OFFSET_TIMESCALE", Signature("F3 0F 59 6F ?? F3 44 0F 59 0D [ ?? ?? ?? ?? ]"));
  m_Signatures.emplace("OFFSET_CAMERAUPDATE", Signature("F3 0F 10 4E ?? 48 8B CF E8 [ ?? ?? ?? ?? ] EB 0B"));
  //m_Signatures.emplace("OFFSET_CAMERAUPDATE2", Signature("F3 0F 10 4E ?? 48 8B CF E8 [ ?? ?? ?? ?? ] EB 0B", 0x1200));
  m_Signatures.emplace("OFFSET_INPUTUPDATE", Signature("E8 [ ?? ?? ?? ?? ] 48 8B 0D ?? ?? ?? ?? 48 85 C9 74 0A F3 0F 10 4B"));

  util::log::Write("Scanning for offsets...");

  MODULEINFO info;
  if (!GetModuleInformation(GetCurrentProcess(), g_gameHandle, &info, sizeof(MODULEINFO)))
  {
    util::log::Error("GetModuleInformation failed, GetLastError 0x%X", GetLastError());
    util::log::Error("Offset scanning unavailable");
    return;
  }

  bool allFound = true;

  for (auto& entry : m_Signatures)
  {
    Signature& sig = entry.second;
    __int64 result = (__int64)FindPattern((BYTE*)info.lpBaseOfDll, info.SizeOfImage, sig.Pattern, sig.Mask.c_str());
    if (!result)
    {
      util::log::Error("Could not find pattern for %s", entry.first.c_str());
      allFound = false;
      continue;
    }

    if (sig.HasReference)
    {
      // Get the assembly reference
      int* pReference = (int*)(result + sig.ReferenceOffset);
      // Assembly reference is relative to the address after the reference
      sig.Result = ((__int64)pReference + sig.ReferenceSize) + *pReference;
      sig.Result += sig.AddOffset;
    }
    else
      sig.Result = result + sig.AddOffset;

    sig.Result = sig.Result - (__int64)g_gameHandle;
  }

  if (allFound)
    util::log::Ok("All offsets found");
  else
    util::log::Warning("All offsets could not be found, this might result in a crash");

  std::fstream file;
  file.open("./Cinematic Tools/Offsets.log", std::fstream::in | std::fstream::out | std::fstream::trunc);
  for (auto& sig : m_Signatures)
    file << "(\"" + sig.first + "\", \t\t0x" << std::hex << std::uppercase << sig.second.Result << " )\n";

  m_UseScannedResults = true;
}

__int64 util::offsets::GetOffset(std::string const& name)
{
  // If a scan was done, prefer those results.
  // If something couldn't be found or there was no scan,
  // use the hardcoded offsets.

  if (m_UseScannedResults)
  {
    auto result = m_Signatures.find(name);
    if (result != m_Signatures.end())
    {
      if (result->second.Result)
        return result->second.Result + (__int64)g_gameHandle;
    }
  }

  // If the offsets were scanned, their absolute position is known.
  // With hardcoded offsets, use relative offset because it's not
  // 100% guaranteed the game module will load at the same address space.

  auto hardcodedResult = m_HardcodedOffsets.find(name);
  if (hardcodedResult != m_HardcodedOffsets.end())
    return hardcodedResult->second + (__int64)g_gameHandle;

  util::log::Error("Offset %s does not exist", name.c_str());
  return 0;
}