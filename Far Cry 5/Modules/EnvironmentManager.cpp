#include "EnvironmentManager.h"
#include "../Util/ImGuiHelpers.h"
#include "../Main.h"

EnvironmentManager::EnvironmentManager() :
  m_CustomTimeOfDay(1200),
  m_OverrideTimeOfDay(false),
  m_IsLODBackedUp(false),
  m_FixLOD(false)
{

}

EnvironmentManager::~EnvironmentManager()
{

}

void EnvironmentManager::Update()
{
  FC::CRenderGeometryConfig* pConfig = FC::CRenderGeometryConfig::GetActiveConfig();
  if (!pConfig) return;
  if (!m_FixLOD) return;

  if (!m_IsLODBackedUp)
  {
    m_IsLODBackedUp = true;
    m_LODBackup = *pConfig;
  }

  pConfig->m_KillLodScale = 0.25f;
  pConfig->m_LodScale = 0;
}

void EnvironmentManager::DrawUI()
{
  FC::CDynamicEnvironment* pEnvironment = FC::CDynamicEnvironment::Singleton();
  if (!pEnvironment) return;

  ImGuiIO& io = ImGui::GetIO();
  ImGui::PushFont(io.Fonts->Fonts[4]);
  ImGui::Dummy(ImVec2(0, 10));
  ImGui::Columns(4, "VisualColumns", false);
  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 12);

  ImGui::PushItemWidth(200);
  DepthOfField& dof = g_mainHandle->GetCameraManager()->GetDoF();
  ImGui::Text("DoF Focus Distance");
  ImGui::InputFloat("##DofFocus", &dof.focusDistance, 0.1, 1, 2);
  ImGui::Text("DoF Near");
  ImGui::InputFloat("##DofNear", &dof.nearDistance, 0.1, 1, 2);
  ImGui::Text("DoF Far");
  ImGui::InputFloat("##DofFar", &dof.farDistance, 0.1, 1, 2);
  ImGui::Text("DoF CoC size");
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 10));
  ImGui::InputFloat("##DofCoC", &dof.cocSize, 0.05, 0.5, 3);
  ImGui::Checkbox("Enable Depth of Field", &dof.enabled);
  ImGui::PopStyleVar();

  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 290);
  ImGui::PushItemWidth(200);

  ImGui::Text("Time of Day");
  if (ImGui::InputInt("##TimeOfDay", &m_CustomTimeOfDay, 1, 10))
  {
    int hours = m_CustomTimeOfDay / 100;
    int minutes = m_CustomTimeOfDay - (hours * 100);

    if (minutes >= 60 && minutes < 70)
    {
      hours += 1;
      minutes = 0;
    }
    else if (minutes > 70 || minutes < 0)
    {
      if (minutes < 0)
        hours = 23;
      minutes = 59;
    }

    if (hours >= 24)
      hours = 0;
    else if (hours < 0)
      hours = 23;

    m_CustomTimeOfDay = hours * 100 + minutes;
    if (m_OverrideTimeOfDay)
      pEnvironment->m_TimeOfDay = (hours * 60 + minutes) * 60;
  }
  ImGui::Text("Timescale");
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 10));
  ImGui::InputFloat("##EnvTimeScale", &pEnvironment->m_TimeScale, 0.1, 0.1, 3);

  if (ImGui::Checkbox("Override time of day", &m_OverrideTimeOfDay))
  {
    if (m_OverrideTimeOfDay)
    {
      int hours = m_CustomTimeOfDay / 100;
      int minutes = m_CustomTimeOfDay - (hours * 100);
      pEnvironment->m_TimeOfDay = (hours * 60 + minutes) * 60;
    }
  }
  ImGui::PopStyleVar();

  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 552);
  ImGui::PushItemWidth(200);

  unsigned int settingsSize = pEnvironment->m_pSettingsCollection.size;
  FC::CEnvironmentSettings* pSettings = &pEnvironment->m_pSettingsCollection.pComponents[settingsSize - 1];
  if (pSettings)
  {
    FC::CEnvironmentExposure* pExposure = pSettings->m_pExposure;
    if (pExposure)
    {
      ImGui::Text("Min exposure (EV)");
      ImGui::InputFloat("##ExposureMin", &pExposure->m_ExposureMin, 0.5f, 1.f, 2);
      ImGui::Text("Max exposure (EV)");
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 10));
      ImGui::InputFloat("##ExposureMax", &pExposure->m_ExposureMax, 0.5f, 1.f, 2);
      if (ImGui::Checkbox("Apply LOD fixes", &m_FixLOD))
      {
        if (!m_FixLOD && m_IsLODBackedUp)
        {
          FC::CRenderGeometryConfig* pConfig = FC::CRenderGeometryConfig::GetActiveConfig();
          if (pConfig)
            *pConfig = m_LODBackup;
        }
      }
      ImGui::PopStyleVar();
    }
  }

  ImGui::PopFont();
}