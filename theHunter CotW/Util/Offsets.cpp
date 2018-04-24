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
  ("OFFSET_CLOCK", 0x1E08370)
  ("OFFSET_ENVIRONMENTGFX", 0x1E08368)
  ("OFFSET_GRAPHICSENGINE", 0x1D7DAE0);

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

  for (int i = 0; i < sig.size(); ++i)
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
        ReferenceOffset = i;
        break;
      }
      case ']':
      {
        ReferenceSize = i - ReferenceOffset;
        break;
      }
      case '?':
      {
        Mask += '?';
        // In signature it's clearer to mark one wildcard byte as ??
        // so skip the next character.
        i += 1;
      }
      default:
      {
        Mask += 'x';
        // Process 2 characters into a single byte
        Pattern[i] = (util::CharToByte(sig[i]) << 4) + util::CharToByte(sig[i+1]);
        i += 1;
      }
    }
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

  m_Signatures.emplace("OFFSET_EXAMPLE", Signature("12 34 56 78 [ ?? ?? ?? ?? ] AA BB ?? DF", 0x20));

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
  }

  if (allFound)
    util::log::Ok("All offsets found");
  else
    util::log::Warning("All offsets could not be found, this might result in a crash");

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
        return result->second.Result;
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