#define NOMINMAX
#include "Util.h"
#include "../Main.h"

#include <algorithm>
#include <codecvt>
#include <locale>

using namespace DirectX;

bool util::CheckVersion(const char* supportedVersion)
{
  return true;

//   fb::BuildInfo* pBuildInfo = fb::BuildInfo::Singleton();
//   if (GetModuleHandleA("NeedForSpeedPaybackTrial.exe"))
//   {
//     util::log::Warning("You are running the trial version, some features might not work");
//     return false;
//   }
// 
//   if (std::string(pBuildInfo->getBuildDate()) == std::string(supportedVersion))
//   {
//     util::log::Ok("Current Game Version: %s", pBuildInfo->getBuildDate());
//     util::log::Ok("Supported Game Version: %s", supportedVersion);
//     util::log::Ok("Game versions match. Continuing..\n");
//     return true;
//   }
//   else
//   {
//     util::log::Warning("!! VERSION MISMATCH !!");
//     util::log::Warning("Current Game Version: %s", pBuildInfo->getBuildDate());
//     util::log::Warning("Supported Game Version: %s", supportedVersion);
//     util::log::Warning("Some features may not work until next tool update.");
//     return false;
//   }
}

BYTE util::CharToByte(char c)
{
  BYTE b;
  sscanf_s(&c, "%hhx", &b);
  return b;
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

BOOL util::WriteMemory(DWORD_PTR dwAddress, const void* cpvPatch, DWORD dwSize)
{
  DWORD dwProtect;
  if (VirtualProtect((void*)dwAddress, dwSize, PAGE_READWRITE, &dwProtect)) //Unprotect the memory
    memcpy((void*)dwAddress, cpvPatch, dwSize); //Write our patch
  else
    return false; //Failed to unprotect, so return false..

  return VirtualProtect((void*)dwAddress, dwSize, dwProtect, new DWORD); //Reprotect the memory
}

std::string util::VkToString(DWORD vk)
{
  unsigned int scanCode = MapVirtualKey(vk, MAPVK_VK_TO_VSC);

  switch (vk)
  {
  case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN: // arrow keys
  case VK_PRIOR: case VK_NEXT: // page up and page down
  case VK_END: case VK_HOME:
  case VK_INSERT: case VK_DELETE:
  case VK_DIVIDE: // numpad slash
  case VK_NUMLOCK:
  {
    scanCode |= 0x100; // set extended bit
    break;
  }
  }

  wchar_t wKeyName[50];
  char cKeyName[50];
  memset(wKeyName, 0, 100);

  int length = GetKeyNameTextW(scanCode << 16, (LPWSTR)&wKeyName, 50);
  wKeyName[length] = L'\0';

  std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
  return utf8_conv.to_bytes(wKeyName);
}

std::string util::KeyLparamToString(LPARAM lparam)
{
  unsigned int scanCode = (lparam >> 16) & 0xFF;
  if ((lparam >> 24) & 1)
    scanCode |= 0x100;

  wchar_t wKeyName[50];
  char cKeyName[50];
  memset(wKeyName, 0, 100);

  int length = GetKeyNameTextW(scanCode << 16, (LPWSTR)&wKeyName, 50);
  wKeyName[length] = L'\0';

  std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
  return utf8_conv.to_bytes(wKeyName);
}


double clamp(double x, double upper, double lower)
{
  return std::min(upper, std::max(x, lower));
}

// https://en.wikipedia.org/wiki/Slerp#Source_Code
XMVECTOR util::math::QuatSlerp(XMVECTOR q1, XMVECTOR q2, double t)
{
  // Compute the cosine of the angle between the two vectors.
  double dot = XMVector4Dot(q1, q2).m128_f32[0];

  const double DOT_THRESHOLD = 0.9995;
  if (fabs(dot) > DOT_THRESHOLD) {
    // If the inputs are too close for comfort, linearly interpolate
    // and normalize the result.

    XMVECTOR result = q1 + t*(q2 - q1);
    return XMVector4Normalize(result);
  }

  // If the dot product is negative, the quaternions
  // have opposite handed-ness and slerp won't take
  // the shorter path. Fix by reversing one quaternion.
  if (dot < 0.0f) {
    q2 = -q2;
    dot = -dot;
  }

  dot = clamp(dot, 1, -1);           // Robustness: Stay within domain of acos()
  double theta_0 = acos(dot);  // theta_0 = angle between input vectors
  double theta = theta_0*t;    // theta = angle between v0 and result 

  XMVECTOR q3 = q2 - q1*dot;
  q3 = XMVector4Normalize(q3);              // { v0, v2 } is now an orthonormal basis

  return q1*cos(theta) + q3*sin(theta);
}

void util::math::QuatToEuler(float& pitch, float& yaw, float& roll, const DirectX::XMVECTOR& quaternion)
{
  XMFLOAT4 q;
  XMStoreFloat4(&q, quaternion);
  pitch = atan2(2 * (q.y*q.z + q.w*q.x), q.w*q.w - q.x*q.x - q.y*q.y + q.z*q.z);
  yaw = asin(-2 * (q.x*q.z - q.w*q.y));
  roll = atan2(2 * (q.x*q.y + q.w*q.z), q.w*q.w + q.x*q.x - q.y*q.y - q.z*q.z);
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