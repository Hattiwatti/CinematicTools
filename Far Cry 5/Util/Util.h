#pragma once

#include <DirectXMath.h>
#include <string>
#include <Windows.h>

namespace util
{
  namespace hooks
  {
    void Init();

    void DisableHooks();
    void EnableHooks();

    void RemoveHooks();
    void Uninitialize();
  };

  namespace log
  {
    void Init();

    void Write(const char* format, ...);
    void Warning(const char* format, ...);
    void Error(const char* format, ...);
    void Ok(const char* format, ...);
  };

  namespace offsets
  {
    class Signature
    {
    public:
      Signature(std::string, std::string, int);

      PBYTE m_pattern;
      PCHAR m_mask;

      int m_isFunction;

      int m_offset;
      int m_size;

      std::string m_name;
      __int64 m_result;
    };

    void Init();
    __int64 GetOffset(std::string);
    __int64 GetVTable(std::string);
    __int64 FindVTable(std::string);
  };

  bool CheckVersion(const char*);
  BYTE CharToByte(char);
  bool GetResource(int, void*&, DWORD&);
  BOOL WriteMemory(DWORD_PTR, const void*, DWORD);

  std::string VkToString(DWORD vk);
  std::string KeyLparamToString(LPARAM lparam);

  namespace math
  {
    double CatmullRomInterpolate(double y0, double y1, double y2, double y3, double mu);
    DirectX::XMVECTOR QuatSlerp(DirectX::XMVECTOR q1, DirectX::XMVECTOR q2, double t);
    void QuatToEuler(float& pitch, float& yaw, float& roll, const DirectX::XMVECTOR& q);
    void FbxEulerToFbQuat(DirectX::XMVECTOR& q, const float& pitch, const float& yaw, const float& roll);
  }

}