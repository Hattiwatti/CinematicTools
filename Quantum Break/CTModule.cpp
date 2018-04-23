#include "CTModule.h"

void CTModule::HotkeyUpdate()
{
  onHotkeyUpdate();
}

void CTModule::MapChange()
{
  onMapChange();
}

void CTModule::Update()
{
  high_resolution_clock::time_point currentTime = high_resolution_clock::now();
  duration<double> dt = currentTime - m_dtUpdate;
  m_dtUpdate = currentTime;

  onUpdate(dt.count());
}
