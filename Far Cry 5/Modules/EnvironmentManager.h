#pragma once
#include "../Dunya.h"

class EnvironmentManager
{
public:
  EnvironmentManager();
  ~EnvironmentManager();

  void Update();
  void DrawUI();

private:
  int m_CustomTimeOfDay;
  bool m_OverrideTimeOfDay;
  bool m_FixLOD;

  bool m_IsLODBackedUp;
  FC::CRenderGeometryConfig m_LODBackup;

public:
  EnvironmentManager(EnvironmentManager const&) = delete;
  void operator=(EnvironmentManager const&) = delete;
};