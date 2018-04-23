#include "Util.h"
#include "../Main.h"

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

double util::math::CatmullRomInterpolate(double y0, double y1, double y2, double y3, double mu)
{
  double mu2 = mu * mu;
  double a0 = -0.5 * y0 + 1.5 * y1 - 1.5 * y2 + 0.5 * y3;
  double a1 = y0 - 2.5 * y1 + 2 * y2 - 0.5 * y3;
  double a2 = -0.5 * y0 + 0.5 * y2;
  double a3 = y1;

  return a0 * mu * mu2 + a1 * mu2 + a2 * mu + a3;
}
