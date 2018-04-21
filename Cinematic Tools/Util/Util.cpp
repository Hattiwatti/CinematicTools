#include "Util.h"
#include "../Main.h"

// Loads resource data from the .dll file based on resource IDs in resource.h
bool util::GetResource(int ID, void* &pData, DWORD& size)
{
  HRSRC rc = FindResource(g_dllHandle, MAKEINTRESOURCE(ID), RT_RCDATA);
  if (!rc)
  {
    util::log::Error("FindResource failed, GetLastError 0x%X resource ID %d", GetLastError(), ID);
    return false;
  }

  HGLOBAL hglobal = LoadResource(g_dllHandle, rc);
  if (!hglobal)
  {
    util::log::Error("LoadResource failed, GetLastError 0x%X resource ID %d", GetLastError(), ID);
    return false;
  }

  pData = LockResource(hglobal);
  size = SizeofResource(g_dllHandle, rc);

  if (pData == nullptr || size == 0)
  {
    util::log::Error("GetResource returned empty data, GetLastError 0x%X resource ID %d", GetLastError(), ID);
    return false;
  }

  return true;
}
