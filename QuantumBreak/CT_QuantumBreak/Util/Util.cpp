#include "Util.h"
#include "../Main.h"

PBYTE WINAPI util::HookVTableFunction(PDWORD64* ppVTable, PBYTE pHook, SIZE_T iIndex)
{
  DWORD dwOld = 0;
  VirtualProtect((void*)((*ppVTable) + iIndex), sizeof(PDWORD64), PAGE_EXECUTE_READWRITE, &dwOld);

  PBYTE pOrig = ((PBYTE)(*ppVTable)[iIndex]);
  (*ppVTable)[iIndex] = (DWORD64)pHook;

  VirtualProtect((void*)((*ppVTable) + iIndex), sizeof(PDWORD64), dwOld, &dwOld);

  return pOrig;
}

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
    return false;

  return true;
}