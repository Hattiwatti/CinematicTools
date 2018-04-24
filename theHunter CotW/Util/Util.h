#pragma once

#include <DirectXMath.h>
#include <string>
#include <vector>
#include <Windows.h>

namespace util
{
  namespace hooks
  {
    enum HookType
    {
      MinHook,
      VTable
    };

    struct Hook
    {
      __int64 Address;
      unsigned int Index;
      LPVOID* Original;
      HookType Type;
      bool Enabled;
    };

    void Init();

    // if name is empty, then perform on all hooks
    void SetHookState(bool enabled, std::string const& name = "");
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
    struct Signature
    {
      BYTE* Pattern{ nullptr }; // The pattern to search
      std::string Mask;  // Which bytes should be evaluated (x = evaluate, ? = skip)
      
      bool HasReference{ false }; // Interpret the offset from the assembly reference
      int ReferenceOffset{ 0 }; // How far in the signature is the assembly reference
      int ReferenceSize{ 0 }; // How many bytes is the assembly reference (usually 4, obsolete?)
      int AddOffset{ 0 }; // How much bytes should be added to the final result

      __int64 Result;

      Signature(std::string const& sig, int offset = 0);
    };

    void Scan();
    __int64 GetOffset(std::string const& name);
  }

  bool GetResource(int, void*&, DWORD&);
  std::string VkToString(DWORD vk);
  std::string KeyLparamToString(LPARAM lparam);
  BYTE CharToByte(char c);

  namespace math
  {
    double CatmullRomInterpolate(double y0, double y1, double y2, double y3, double mu);
  }
}