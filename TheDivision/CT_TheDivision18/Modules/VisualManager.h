#pragma once

class VisualManager
{
  struct DOFSettings
  {
    bool enable{ false };
    float focusDistance{ 5.0f };
    float fstop{ 3.f };
    float nearDistance{ 7.f };
    float nearFadeInDistance{ 1.f };
    float minCoC{ 0.f };
    float maxCoC{ 20.f };
    float farDistance{ 10.f };
    float farFadeInDistance{ 200.f };
  };

public:
  VisualManager();
  ~VisualManager();

  void DOFHook(__int64 a1);
  void DrawUI();

private:
  DOFSettings m_dofSettings;

  int m_customTimeOfDay;
  bool m_overrideTimeOfDay;

  __int64* m_envPresetArray;
  const char** m_envNameArray;

  int m_selectedCurrentWeather;
  int m_selectedNextWeather;
  int m_environmentCount;

public:
  VisualManager(VisualManager const&) = delete;
  void operator=(VisualManager const&) = delete;
};