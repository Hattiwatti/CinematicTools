#pragma once

#include <boost/chrono.hpp>
using namespace boost::chrono;

class CTModule
{
public:
  CTModule() { };
  CTModule(const char* name) { m_name = name; }
  ~CTModule() { };

  void HotkeyUpdate();
  void MapChange();
  void Update();

  const char* GetName() { return m_name; }

private:
  virtual void onHotkeyUpdate() {};
  virtual void onMapChange() {};
  virtual void onUpdate(double) {};

private:
  boost::chrono::high_resolution_clock::time_point m_dtUpdate;
  const char* m_name;

public:
  CTModule(CTModule const&) = delete;
  void operator=(CTModule const&) = delete;
};