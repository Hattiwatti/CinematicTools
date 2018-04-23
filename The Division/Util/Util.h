#pragma once
#include <vector>
#include <Windows.h>

namespace util
{
  namespace hooks
  {
    void Init();

    void DisableHooks();
    void EnableHooks();
  };

  namespace log
  {
    void Init();

    void Write(const char* format, ...);
    void Warning(const char* format, ...);
    void Error(const char* format, ...);
    void Ok(const char* format, ...);
  };

  bool GetResource(int, void*&, DWORD&);
  namespace math
  {
    double CatmullRomInterpolate(double y0, double y1, double y2, double y3, double mu);
  }
}