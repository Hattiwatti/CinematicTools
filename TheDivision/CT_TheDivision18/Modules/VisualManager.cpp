#include "VisualManager.h"
#include "Snowdrop.h"
#include "../Util/Util.h"
#include "../imgui/imgui.h"

VisualManager::VisualManager()
{
  m_overrideTimeOfDay = false;
  m_customTimeOfDay = 1200;

  TD::EnvironmentFileSystem* pEnvFiles = TD::EnvironmentFileSystem::Singleton();
  m_envPresetArray = new __int64[pEnvFiles->m_handleCount];
  m_envNameArray = new const char*[pEnvFiles->m_handleCount];

  for (int i = 0; i < pEnvFiles->m_handleCount; ++i)
  {
    m_envPresetArray[i] = pEnvFiles->m_pHandles[i].pEntity;
    std::string sName(*(const char**)(pEnvFiles->m_pHandles[i].pEntity + 0x10));

    sName = sName.substr(18, sName.length() - 18 - 13);
    m_envNameArray[i] = new const char[sName.length()+1];
    memcpy((void*)m_envNameArray[i], sName.c_str(), sName.length()+1);
  }

  m_selectedCurrentWeather = 0;
  m_selectedNextWeather = 0;
  m_environmentCount = pEnvFiles->m_handleCount;
}

VisualManager::~VisualManager()
{

}

struct DOFStructure
{
  int enable1;
  int enable2;
  float fstop;
  float focusDistance;
  float minCoC;
  float maxCoC;
};

void VisualManager::DOFHook(__int64 a1)
{
  if (!m_dofSettings.enable) return;

  __int64 ptr1 = a1 + 0x70;
  __int64 ptr2 = *(__int64*)(ptr1 + 0x1F38);

  int ptrMultiplier = *(int*)(ptr1 + 0x1F4C);
  DOFStructure* pDoF = (DOFStructure*)((ptr2 + 0x7800 * ptrMultiplier) + 0x66F0);

  TD::HudSettings* pHudSettings = TD::RogueClient::Singleton()->m_pClient->m_pWorld->m_pDoF;
  pHudSettings->m_Timer = 0xFF;
  pHudSettings->m_CloseUpEffectsDistance = m_dofSettings.nearDistance;
  pHudSettings->m_CloseUpEffectsFadeInDistance = m_dofSettings.nearFadeInDistance;
  pHudSettings->m_DOFFStop = m_dofSettings.fstop;
  pHudSettings->m_MinCoC = m_dofSettings.minCoC;
  pHudSettings->m_MaxCoC = m_dofSettings.maxCoC;

  pDoF->enable1 = 1;
  pDoF->enable2 = 1;
  pDoF->focusDistance = m_dofSettings.focusDistance;
  pDoF->fstop = m_dofSettings.fstop;
}

void VisualManager::DrawUI()
{
  TD::EnvironmentManager* pEnvManager = TD::RogueClient::Singleton()->m_pClient->m_pWorld->m_pEnvironmentManager;
  ImGuiIO& io = ImGui::GetIO();

  ImGui::PushFont(io.Fonts->Fonts[4]);
  ImGui::Dummy(ImVec2(0, 10));
  ImGui::Columns(4, "VisualColumns", false);
  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 12);

  ImGui::PushItemWidth(200);

  ImGui::Text("Focus distance");
  ImGui::InputFloat("##DoFFocusDistance", &m_dofSettings.focusDistance, 0.5, 1.0, 3);
  ImGui::Text("F-Stop");
  ImGui::InputFloat("##DoFFStop", &m_dofSettings.fstop, 0.1, 1.0, 2);
  ImGui::Text("Min CoC");
  ImGui::InputFloat("##DoFMinCoC", &m_dofSettings.minCoC, 1, 1.0, 2);
  ImGui::Text("Max CoC");
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 10));
  ImGui::InputFloat("##DoFMaxCoC", &m_dofSettings.maxCoC, 1, 1.0, 2);
  ImGui::Checkbox("Enable DoF", &m_dofSettings.enable);
  ImGui::PopStyleVar();

  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 290);
  ImGui::PushItemWidth(200);

  ImGui::Text("Current Weather");
  ImGui::Combo("##CurrentWeather", &m_selectedCurrentWeather, m_envNameArray, m_environmentCount);
  ImGui::SameLine();
  if (ImGui::Button("Set##1"))
    pEnvManager->SetCurrentWeather(m_envPresetArray[m_selectedCurrentWeather]);

  ImGui::Text("Next Weather");
  ImGui::Combo("##NextWeather", &m_selectedNextWeather, m_envNameArray, m_environmentCount);
  ImGui::SameLine();
  if (ImGui::Button("Set##2"))
    pEnvManager->SetNextWeather(m_envPresetArray[m_selectedNextWeather]);

  ImGui::Text("Blend factor of current and next weather");
  ImGui::SliderFloat("##BlendFactor", &pEnvManager->m_pEnvironmentValues->m_BlendValue, 0, 1);

  ImGui::Text("Time of Day");
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 10));
  if (ImGui::InputInt("##TimeOfDay", &m_customTimeOfDay, 1, 10))
  {
    int hours = m_customTimeOfDay / 100;
    int minutes = m_customTimeOfDay - (hours * 100);

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

    m_customTimeOfDay = hours * 100 + minutes;
    if (m_overrideTimeOfDay)
      pEnvManager->m_TimeOfDay = (hours * 60 + minutes) * 60 * 1000;
  }

  if (ImGui::Checkbox("Override time of day", &m_overrideTimeOfDay))
  {
    pEnvManager->m_FreezeToD = m_overrideTimeOfDay;
    if (m_overrideTimeOfDay)
    {
      int hours = m_customTimeOfDay / 100;
      int minutes = m_customTimeOfDay - hours;
      pEnvManager->m_TimeOfDay = (hours * 60 + minutes) * 60 * 1000;
    }
  }
  ImGui::PopStyleVar();

  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 552);
  ImGui::PushItemWidth(200);

  ImGui::Text("Timer state (ms)");
  ImGui::InputInt("##TimerState", &pEnvManager->m_WeatherTimer, 1000, 1000);
  ImGui::Text("Timer end (ms)");
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 10));
  ImGui::InputInt("##TimerEnd", &pEnvManager->m_WeatherTimerMax, 1000, 1000);
  ImGui::Checkbox("Run timer", &pEnvManager->m_RunWeatherTimer);

  ImGui::PopStyleVar();
  ImGui::PopFont();
}