#pragma once
#include "../AlienIsolation.h"

class VisualsController
{
public:
  VisualsController();
  ~VisualsController();

  void Update();

  void OnHotkeyUpdate();
  void OnPostProcessUpdate(CATHODE::PostProcess*);
  void OnTonemapUpdate();

  void DrawUI();

private:
  CATHODE::PostProcess m_PostProcess;
  CATHODE::DayToneMapSettings m_Tonemap;

  bool m_PostProcessInitialized;
  bool m_TonemapInitialized;
  bool m_Override;

  bool m_UIRequestReset;

public:
  VisualsController(VisualsController const&) = delete;
  void operator=(VisualsController const&) = delete;
};