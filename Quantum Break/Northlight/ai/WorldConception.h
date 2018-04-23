#pragma once
#include <Windows.h>

namespace ai
{
  class WorldConception
  {
  public:
    BYTE Pad000[0xA10];
    float m_position[3];
  };
}