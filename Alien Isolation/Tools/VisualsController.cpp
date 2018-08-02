#include "VisualsController.h"
#include "../Main.h"
#include "../Util/ImGuiEXT.h"

VisualsController::VisualsController() : 
  m_PostProcessInitialized(false),
  m_TonemapInitialized(false),
  m_Override(false)
{

}

VisualsController::~VisualsController()
{

}

void VisualsController::Update()
{
  if (m_UIRequestReset)
  {
    m_UIRequestReset = false;
    bool overrideState = m_Override;

    m_Override = false;
    Sleep(100);

    m_PostProcessInitialized = false;
    m_TonemapInitialized = false;

    m_Override = overrideState;
  }
}

void VisualsController::OnHotkeyUpdate()
{

}

void VisualsController::OnPostProcessUpdate(CATHODE::PostProcess* pPostProcess)
{
  if (!m_PostProcessInitialized)
  {
    m_PostProcess = *pPostProcess;
    m_PostProcessInitialized = true;
  }

  if (!m_Override) return;

  pPostProcess->m_Gamma = m_PostProcess.m_Gamma;
  pPostProcess->m_Brightness = m_PostProcess.m_Brightness;
  pPostProcess->m_Contrast = m_PostProcess.m_Contrast;
  pPostProcess->m_Saturation = m_PostProcess.m_Saturation;
  pPostProcess->m_TintColor = m_PostProcess.m_TintColor;
}

void VisualsController::OnTonemapUpdate()
{
  CATHODE::DayToneMapSettings* pTonemap = CATHODE::PostProcessSystem::Singleton()->m_TonemapSettings;
  if (!pTonemap) return;

  if (!m_TonemapInitialized)
  {
    m_Tonemap = *pTonemap;
    m_TonemapInitialized = true;
  }

  if (!m_Override) return;

  pTonemap->m_BlackPoint = m_Tonemap.m_BlackPoint;
  pTonemap->m_WhitePoint = m_Tonemap.m_WhitePoint;
  pTonemap->m_CrossOverPoint = m_Tonemap.m_CrossOverPoint;
  pTonemap->m_ToeStrength = m_Tonemap.m_ToeStrength;
  pTonemap->m_ShoulderStrength = m_Tonemap.m_ShoulderStrength;
  pTonemap->m_LuminanceScale = m_Tonemap.m_LuminanceScale;
}

void VisualsController::DrawUI()
{
  ImGuiIO& io = ImGui::GetIO();

  ImGui::Columns(2, 0, false);
  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 12);

  ImGui::Dummy(ImVec2(0, 10));

  ImGui::PushFont(io.Fonts->Fonts[4]);
  ImGui::PushItemWidth(200);
  ImGui::Checkbox("Override values", &m_Override); ImGui::SameLine();
  ImGui::DrawWithBorders([=] { 
    m_UIRequestReset |= ImGui::Button("Reset values"); 
  });

  ImGui::Text("Color tint");
  ImGui::ColorEdit3("##ColorTint", &m_PostProcess.m_TintColor.x);

  ImGui::Text("Brightness"); ImGui::SameLine(206);
  ImGui::Text("Contrast"); ImGui::SameLine(415);
  ImGui::Text("Saturation"); ImGui::SameLine(623);
  ImGui::Text("Gamma");

  ImGui::InputFloat("##Brightness", &m_PostProcess.m_Brightness); ImGui::SameLine();
  ImGui::InputFloat("##Contrast", &m_PostProcess.m_Contrast); ImGui::SameLine();
  ImGui::InputFloat("##Saturation", &m_PostProcess.m_Saturation); ImGui::SameLine();
  ImGui::InputFloat("##Gamma", &m_PostProcess.m_Gamma);

  ImGui::Dummy(ImVec2(0, 30));
  ImGui::PushFont(io.Fonts->Fonts[3]);
  ImGui::Text("Tonemap settings");
  ImGui::PopFont();

  ImGui::Text("Black point"); ImGui::SameLine(206);
  ImGui::Text("Crossover point"); ImGui::SameLine(415);
  ImGui::Text("White point");
  ImGui::InputFloat("##BlackPoint", &m_Tonemap.m_BlackPoint); ImGui::SameLine();
  ImGui::InputFloat("##CrossOverPoint", &m_Tonemap.m_CrossOverPoint); ImGui::SameLine();
  ImGui::InputFloat("##WhitePoint", &m_Tonemap.m_WhitePoint);

  ImGui::Text("Toe strength"); ImGui::SameLine(206);
  ImGui::Text("Shoulder strength"); ImGui::SameLine(415);
  ImGui::Text("Luminance scale");
  ImGui::InputFloat("##ToeStrength", &m_Tonemap.m_ToeStrength); ImGui::SameLine();
  ImGui::InputFloat("##ShoulderStrength", &m_Tonemap.m_ShoulderStrength); ImGui::SameLine();
  ImGui::InputFloat("##LuminanceScale", &m_Tonemap.m_LuminanceScale);


  ImGui::PopFont();
}


