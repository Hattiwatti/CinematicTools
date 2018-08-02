#include "CameraManager.h"
#include "../Main.h"
#include "../Util/ImGuiEXT.h"
#include "../inih/cpp/INIReader.h"

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <iostream>
#include <Windows.h>

// Helper for ImGui combo
static auto ProfileNameGetter = [](void* vec, int idx, const char** out_text)
{
  std::vector<CameraProfile>* v = reinterpret_cast<std::vector<CameraProfile>*>(vec);
  *out_text = v->at(idx).Name.c_str();
  return true;
};

CameraManager::CameraManager() :
  m_CameraEnabled(false),
  m_AutoReset(false),
  m_FirstEnable(true),
  m_UIRequestReset(false),
  m_GamepadDisabled(true),
  m_KbmDisabled(true),
  m_SmoothMouse(true),
  m_Camera(),
  m_TrackPlayer(),
  m_CharacterIndex(0),
  m_LockToCharacter(false),
  m_pCharacter(nullptr),
  m_HideUI(false),
  m_ShowProfileModal(false),
  m_ModalProfileName("New profile\0"),
  m_SelectedProfile(0),
  m_TimeScale(1.f)
{
  
}

CameraManager::~CameraManager()
{

}

void CameraManager::HotkeyUpdate()
{
  InputSystem* pInput = g_mainHandle->GetInputSystem();

  if (pInput->IsActionDown(Action::ToggleCamera))
  {
    ToggleCamera();
    while (pInput->IsActionDown(Action::ToggleCamera))
      Sleep(100);
  }

  if (pInput->IsActionDown(Action::ToggleHUD))
  {
    ToggleHUD();

    while (pInput->IsActionDown(Action::ToggleHUD))
      Sleep(100);
  }

  if (pInput->IsActionDown(Action::ToggleFreezeTime))
  {
    bool* pFreezeTime = (bool*)(*(bool**)util::offsets::GetOffset("OFFSET_FREEZETIME"));
    *pFreezeTime = !*pFreezeTime;

    while (pInput->IsActionDown(Action::ToggleFreezeTime))
      Sleep(100);
  }

  if (m_CameraEnabled)
  {
    if (pInput->IsActionDown(Action::Track_CreateNode))
    {
      m_TrackPlayer.CreateNode(m_Camera);

      while (pInput->IsActionDown(Action::Track_CreateNode))
        Sleep(100);
    }

    if (pInput->IsActionDown(Action::Track_DeleteNode))
    {
      m_TrackPlayer.DeleteNode();

      while (pInput->IsActionDown(Action::Track_DeleteNode))
        Sleep(100);
    }

    if (pInput->IsActionDown(Action::Track_Play))
    {
      m_TrackPlayer.Toggle();

      while (pInput->IsActionDown(Action::Track_Play))
        Sleep(100);
    }
  }
}

XMFLOAT4 savedRotations[3];
XMFLOAT3 savedPositions[3];

void CameraManager::OnCameraUpdateBegin()
{
  if (!m_CameraEnabled) return;

  // Final camera position and rotation calculations are done here
  // because character locked cameras stutter if they are updated
  // outside the game thread.

  XMMATRIX targetMatrix = m_LockToCharacter ? GetTargetMatrix() : XMMatrixIdentity();
  XMVECTOR targetRotation = XMQuaternionRotationMatrix(targetMatrix);
  XMVECTOR vPosition = XMLoadFloat3(&m_Camera.Position);
  XMVECTOR qRotation = XMLoadFloat4(&m_Camera.Rotation);

  XMVECTOR finalPosition = targetMatrix.r[3];
  finalPosition += targetMatrix.r[0] * vPosition.m128_f32[0];
  finalPosition += targetMatrix.r[1] * vPosition.m128_f32[1];
  finalPosition += targetMatrix.r[2] * vPosition.m128_f32[2];

  XMVECTOR finalRotation = XMQuaternionMultiply(qRotation, targetRotation);

  XMStoreFloat3(&m_Camera.AbsolutePosition, finalPosition);
  XMStoreFloat4(&m_Camera.AbsoluteRotation, finalRotation);

  CATHODE::AICamera* pCamera = CATHODE::Main::Singleton()->m_CameraManager->m_ActiveCamera;
  for (int i = 0; i < 3; ++i)
  {
    savedRotations[i] = pCamera->m_State[i].m_Rotation;
    savedPositions[i] = pCamera->m_State[i].m_Position;

    XMStoreFloat3(&pCamera->m_State[i].m_Position, finalPosition);
    XMStoreFloat4(&pCamera->m_State[i].m_Rotation, finalRotation);
    pCamera->m_State[i].m_FieldOfView = m_Camera.Profile.FieldOfView;
  }
}

void CameraManager::OnCameraUpdateEnd()
{
  if (!m_CameraEnabled) return;

  CATHODE::AICamera* pCamera = CATHODE::Main::Singleton()->m_CameraManager->m_ActiveCamera;
  for (int i = 0; i < 3; ++i)
  {
    pCamera->m_State[i].m_Rotation = savedRotations[i];
    pCamera->m_State[i].m_Position = savedPositions[i];
  }
}

void CameraManager::OnPostProcessUpdate(CATHODE::PostProcess* pPostProcess)
{
  if (!m_CameraEnabled) return;
  pPostProcess->m_DofFocusDistance = m_Camera.Profile.FocusDistance;
  pPostProcess->m_DofStrength = m_Camera.Profile.DofStrength;
  pPostProcess->m_DofScale = m_Camera.Profile.DofScale;
}

void CameraManager::OnMapChange()
{

}

void CameraManager::Update(float dt)
{
  if (!m_CameraEnabled) return;
  if (m_UIRequestReset) ResetCamera();

  UpdateInput(dt);
  UpdateCamera(dt);
}

void CameraManager::DrawUI()
{
  ImGuiIO& io = ImGui::GetIO();

  ImGui::Dummy(ImVec2(0, 10));
  ImGui::Dummy(ImVec2(448, 0));

  ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 1, 1, 1));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
  ImGui::PushFont(io.Fonts->Fonts[3]);
  ImGui::SameLine();
  if (ImGui::ToggleButton(m_CameraEnabled ? "Disable" : "Enable", ImVec2(100, 30), m_CameraEnabled, true))
    ToggleCamera();
  ImGui::SameLine();
  m_UIRequestReset |= ImGui::Button("Reset", ImVec2(100, 30));

  ImGui::PopFont();
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();
  ImGui::PushFont(io.Fonts->Fonts[4]);

  ImGui::Dummy(ImVec2(0, 10));
  ImGui::Columns(5, 0, false);
  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 12);

  ImGui::PushItemWidth(200);
  bool configChanged = false;

  ImGui::Text("Movement speed");
  configChanged |= ImGui::InputFloat("##CameraMovementSpeed", &m_Camera.Profile.MovementSpeed, 0.1f, 1.0f, 2);
  ImGui::Text("Rotation speed");
  configChanged |= ImGui::InputFloat("##CameraRotationSpeed", &m_Camera.Profile.RotationSpeed, 0.1f, 1.0f, 2);
  ImGui::Text("Roll speed");
  configChanged |= ImGui::InputFloat("##CameraRollSpeed", &m_Camera.Profile.RollSpeed, 0.1f, 1.0f, 2);
  ImGui::Text("FoV speed");
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 10));
  configChanged |= ImGui::InputFloat("##CameraFoVSpeed", &m_Camera.Profile.FovSpeed, 0.1f, 1.0f, 2);

  ImGui::Checkbox("Disable player KBM input", &m_KbmDisabled);
  ImGui::Checkbox("Disable player gamepad input", &m_GamepadDisabled);
  configChanged |= ImGui::Checkbox("Reset camera automatically", &m_AutoReset);
  ImGui::Checkbox("Smooth mouse", &m_SmoothMouse);
  ImGui::PopStyleVar();

  /////////////////////////////////////////////////
  ////////////////////////////////////////////////

  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 290);
  ImGui::PushItemWidth(200);

  ImGui::Text("Field of view");
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 10));
  configChanged |= ImGui::InputFloat("##CameraFoV", &m_Camera.Profile.FieldOfView, 1.f, 1.f, 2);
  ImGui::PopStyleVar();

  ImGui::Text("Focus distance");
  configChanged |= ImGui::InputFloat("##FocusDistance", &m_Camera.Profile.FocusDistance, 0.1f, 0.5f, 2);

  ImGui::Text("Focus Scale");
  configChanged |= ImGui::InputFloat("##DofScale", &m_Camera.Profile.DofScale, 0.1f, 0.5f, 2);

  ImGui::Text("DoF Strength");
  configChanged |= ImGui::InputFloat("##DofStrength", &m_Camera.Profile.DofStrength, 0.001f, 0.01f, 3);

  ImGui::Text("Timescale");
 
  if (ImGui::InputFloat("##Timescale", &m_TimeScale, 0.1f, 0, 3))
  {
    if (m_TimeScale < 0)
      m_TimeScale = 0;
  }

  double* pTimescale = reinterpret_cast<double*>(util::offsets::GetOffset("OFFSET_TIMESCALE"));
  *pTimescale = static_cast<double>(m_TimeScale);


  CharacterList const& chrList = g_mainHandle->GetCharacterController()->GetCharacters();
  if (chrList.Count > 0)
  {
    if (m_CharacterIndex >= chrList.Count)
    {
      m_CharacterIndex = chrList.Count - 1;
      m_pCharacter = chrList.Characters[m_CharacterIndex];
    }

    ImGui::Text("Target Character");
    ImGui::Combo("##TargetChr", (int*)&m_CharacterIndex, &chrList.Names[0], chrList.Count);
    if (ImGui::Checkbox("Lock to character", &m_LockToCharacter))
      ChangeCamRelativity();

    m_pCharacter = chrList.Characters[m_CharacterIndex];
  }
  else
    m_pCharacter = nullptr;

  /////////////////////////////////////////////////
  ////////////////////////////////////////////////

  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 552);
  ImGui::PushItemWidth(200);

  m_TrackPlayer.DrawUI();

  /////////////////////////////////////////////////
  ////////////////////////////////////////////////

  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 814);
  ImGui::PushItemWidth(200);

  ImGui::Text("Camera profiles");
  if (ImGui::Combo("##CameraProfile", &m_SelectedProfile, ProfileNameGetter, static_cast<void*>(&m_Profiles), (int)m_Profiles.size()))
    m_Camera.Profile = m_Profiles[m_SelectedProfile];

  if (ImGui::Button("Save profile"))
    ImGui::OpenPopup("CameraProfileModal");

  if (ImGui::BeginPopupModal("CameraProfileModal"))
  {
    ImGui::Text("Profile name");
    ImGui::InputText("##ProfileName", m_ModalProfileName, 50);
    if (ImGui::Button("Save"))
    {
      CreateProfile();
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }

  ImGui::PopFont();

  if (configChanged)
    g_mainHandle->OnConfigChanged();
}

void CameraManager::ReadConfig(INIReader* pReader)
{
  LoadProfiles();
  m_AutoReset = pReader->GetBoolean("Camera", "AutoReset", false);
  
  std::string sSelectedProfile = pReader->Get("Camera", "SelectedProfile", "");
  if (sSelectedProfile.empty()) return;

  for (size_t i = 0; i < m_Profiles.size(); ++i)
  {
    if (m_Profiles[i].Name == sSelectedProfile)
    {
      m_SelectedProfile = static_cast<unsigned int>( i );
      m_Camera.Profile = m_Profiles[i];
      break;
    }
  }
}

const std::string CameraManager::GetConfig()
{
  SaveProfiles();

  std::string config = "[Camera]\n";
  config += "SelectedProfile = " + m_Profiles[m_SelectedProfile].Name + "\n";
  config += "AutoReset = " + std::to_string(m_AutoReset) + "\n";

  return config;
}

void CameraManager::UpdateCamera(float dt)
{
  XMVECTOR qPitch = XMQuaternionRotationRollPitchYaw(-m_Camera.dPitch * dt * m_Camera.Profile.RotationSpeed, 0, 0);
  XMVECTOR qYaw = XMQuaternionRotationRollPitchYaw(0, -m_Camera.dYaw* dt * m_Camera.Profile.RotationSpeed, 0);
  XMVECTOR qRoll = XMQuaternionRotationRollPitchYaw(0, 0, m_Camera.dRoll* dt * m_Camera.Profile.RollSpeed);
  
  XMVECTOR qRotation = XMLoadFloat4(&m_Camera.Rotation);
  XMVECTOR vPosition = XMLoadFloat3(&m_Camera.Position);

  // Adds delta rotations to camera rotation
  qRotation = XMQuaternionMultiply(qPitch, qRotation);
  qRotation = XMQuaternionMultiply(qRotation, qYaw);
  qRotation = XMQuaternionMultiply(qRoll, qRotation);

  // Make sure it's normalized
  qRotation = XMQuaternionNormalize(qRotation);
  
  m_Camera.Profile.FieldOfView += m_Camera.dFov * dt * m_Camera.Profile.FovSpeed;
  m_Camera.Profile.FocusDistance += m_Camera.dFocus * dt * 1;
  m_Camera.Profile.DofScale += m_Camera.dDofScale * dt * 1;
  m_Camera.Profile.DofStrength += m_Camera.dDofStrength * dt * 0.01f;

  // If a camera track is being played, get the current
  // state and overwrite position/rotation/FoV.
  if (m_TrackPlayer.IsPlaying())
  {
    CatmullRomNode resultNode = m_TrackPlayer.PlayForwardSmooth(dt);

    if (m_TrackPlayer.IsRotationLocked())
      qRotation = XMLoadFloat4(&resultNode.Rotation);

    if (m_TrackPlayer.IsFovLocked())
      m_Camera.Profile.FieldOfView = resultNode.FieldOfView;

    if (m_TrackPlayer.IsDofLocked())
    {
      m_Camera.Profile.FocusDistance = resultNode.FocusDistance;
      m_Camera.Profile.DofScale = resultNode.DofScale;
      m_Camera.Profile.DofStrength = resultNode.DofStrength;
    }

    vPosition = XMLoadFloat3(&resultNode.Position);
  }

  // Create rotation matrix
  XMMATRIX rotMatrix = XMMatrixRotationQuaternion(qRotation);

  // Add delta positions
  if (!m_TrackPlayer.IsPlaying())
  {
    vPosition += m_Camera.dX * rotMatrix.r[0] * dt * m_Camera.Profile.MovementSpeed;
    vPosition += m_Camera.dY * rotMatrix.r[1] * dt * m_Camera.Profile.MovementSpeed;
    vPosition -= m_Camera.dZ * rotMatrix.r[2] * dt * m_Camera.Profile.MovementSpeed;
  }

  // Store results
  rotMatrix.r[3] = vPosition;
  rotMatrix.r[3].m128_f32[3] = 1.0f;

  XMStoreFloat3(&m_Camera.Position, vPosition);
  XMStoreFloat4(&m_Camera.Rotation, qRotation);

  m_Camera.dX = 0;
  m_Camera.dY = 0;
  m_Camera.dZ = 0;
  m_Camera.dPitch = 0;
  m_Camera.dYaw = 0;
  m_Camera.dRoll = 0;
  m_Camera.dFov = 0;
  m_Camera.dFocus = 0;
  m_Camera.dDofScale = 0;
  m_Camera.dDofStrength = 0;
}

void CameraManager::UpdateInput(float dt)
{
  InputSystem* pInput = g_mainHandle->GetInputSystem();
  if (!g_hasFocus || g_mainHandle->GetUI()->HasKeyboardFocus())
    return;

  // These need to be changed according to the right/left-handness of game camera
  if (m_KbmDisabled)
  {
    m_Camera.dX = pInput->GetActionState(Camera_Right) - pInput->GetActionState(Camera_Left);
    m_Camera.dY = pInput->GetActionState(Camera_Up) - pInput->GetActionState(Camera_Down);
    m_Camera.dZ = pInput->GetActionState(Camera_Backward) - pInput->GetActionState(Camera_Forward);
  }
  else
  {
    m_Camera.dX = pInput->GetActionState(Camera_RightSecondary) - pInput->GetActionState(Camera_LeftSecondary);
    m_Camera.dY = pInput->GetActionState(Camera_UpSecondary) - pInput->GetActionState(Camera_DownSecondary);
    m_Camera.dZ = pInput->GetActionState(Camera_BackwardSecondary) - pInput->GetActionState(Camera_ForwardSecondary);
  }

  m_Camera.dPitch = pInput->GetActionState(Camera_PitchUp) - pInput->GetActionState(Camera_PitchDown);
  m_Camera.dYaw = pInput->GetActionState(Camera_YawLeft) - pInput->GetActionState(Camera_YawRight);
  m_Camera.dRoll = pInput->GetActionState(Camera_RollLeft) - pInput->GetActionState(Camera_RollRight);
  m_Camera.dFov = pInput->GetActionState(Camera_IncFov) - pInput->GetActionState(Camera_DecFov);
  m_Camera.dFocus = pInput->GetActionState(Visuals_IncFocusDist) - pInput->GetActionState(Visuals_DecFocusDist);
  m_Camera.dDofScale = pInput->GetActionState(Visuals_IncDofScale) - pInput->GetActionState(Visuals_DecDofScale);
  m_Camera.dDofStrength = pInput->GetActionState(Visuals_IncDofStrength) - pInput->GetActionState(Visuals_DecDofStrength);

  // Gamepad controls for movement & rotation speed
  // Hardcoded for now
  m_Camera.Profile.RotationSpeed += (pInput->GetPadKeyState(GamepadKey::DPad_Up) - pInput->GetPadKeyState(GamepadKey::DPad_Down)) * dt;
  m_Camera.Profile.MovementSpeed += (pInput->GetPadKeyState(GamepadKey::Button3) - pInput->GetPadKeyState(GamepadKey::Button4)) * dt;

  if (m_Camera.Profile.RotationSpeed < 0.01f)
    m_Camera.Profile.RotationSpeed = 0.01f;

  if (m_Camera.Profile.MovementSpeed < 0.1f)
    m_Camera.Profile.MovementSpeed = 0.1f;

  if (m_KbmDisabled && !g_mainHandle->GetUI()->IsEnabled())
  {
    XMFLOAT3 state = pInput->GetMouseState();
    float sensitivity = pInput->GetMouseSensitivity();

    m_MouseBuffer.AddValue(state);
    XMFLOAT3 smoothState = m_MouseBuffer.CalcAverage();

    if (m_SmoothMouse)
      state = smoothState;

    m_Camera.dPitch -= state.y * sensitivity;
    m_Camera.dYaw -= state.x * sensitivity;
    m_Camera.dFov += smoothState.z;
  }
}

void CameraManager::ToggleCamera()
{
  // If first enable, fetch game camera location
  if (m_FirstEnable || m_AutoReset)
  {
    XMFLOAT3 pos = CATHODE::Main::Singleton()->m_CameraManager->m_ActiveCamera->m_State[2].m_Position;
    m_Camera.Position = !m_LockToCharacter ? pos : XMFLOAT3(0,0,0);
    util::log::Write("First pos: %.2f %.2f %.2f", m_Camera.Position.x, m_Camera.Position.y, m_Camera.Position.z);
    m_Camera.Rotation = XMFLOAT4(0, 0, 0, 1);
    m_FirstEnable = false;
  }

  m_CameraEnabled = !m_CameraEnabled;
}

void CameraManager::ResetCamera()
{
  m_UIRequestReset = false;
  if (!m_CameraEnabled) return;

  ToggleCamera();
  Sleep(100);
  m_FirstEnable = true;
  ToggleCamera();
}

XMMATRIX CameraManager::GetTargetMatrix()
{
  XMMATRIX result = XMMatrixIdentity();

  if (!m_pCharacter) return result;
  result = XMLoadFloat4x4(&m_pCharacter->m_Transform);

  return result;
}

void CameraManager::ChangeCamRelativity()
{
  if (m_LockToCharacter)
  {
    m_Camera.Position = XMFLOAT3(0, 0, 0);
  }
  else
  {
    CATHODE::AICamera* pCamera = CATHODE::Main::Singleton()->m_CameraManager->m_ActiveCamera;
    m_Camera.Position = pCamera->m_State[2].m_Position;
  }
}

void CameraManager::LoadProfiles()
{
  boost::filesystem::path profileDir("./Cinematic Tools/Profiles/");
  for (auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(profileDir), {}))
  {
    const boost::filesystem::path &profilePath = entry.path();
    
    INIReader reader(profilePath.generic_string().c_str());

    // Make sure the opened file is actually a camera profile
    if (!reader.GetBoolean("CameraProfile", "IsProfile", false))
      continue;

    CameraProfile profile;
    profile.Name = reader.Get("CameraProfile", "Name", "UNKNOWN");
    profile.MovementSpeed = static_cast<float>( reader.GetReal("CameraProfile", "MovementSpeed", 1.0f) );
    profile.RotationSpeed = static_cast<float>( reader.GetReal("CameraProfile", "RotationSpeed", XM_PI / 4) );
    profile.RollSpeed = static_cast<float>( reader.GetReal("CameraProfile", "RollSpeed", XM_PI / 8) );
    profile.FovSpeed = static_cast<float>( reader.GetReal("CameraProfile", "FovSpeed", 5.0f) );
    profile.FieldOfView = static_cast<float>( reader.GetReal("CameraProfile", "FieldOfView", 50.0f) );
    profile.FocusDistance = static_cast<float>( reader.GetReal("CameraProfile", "FocusDistance", 2.0f) );
    profile.DofStrength = static_cast<float>( reader.GetReal("CameraProfile", "DofStrength", 0.04f) );
    profile.DofScale = static_cast<float>( reader.GetReal("CameraProfile", "DofScale", 1.0f) );

    m_Profiles.emplace_back(profile);
  }

  // If there were no profiles, save current one as default
  if (m_Profiles.size() == 0)
    m_Profiles.emplace_back(m_Camera.Profile);
}

void CameraManager::CreateProfile()
{
  CameraProfile profile = m_Camera.Profile;
  profile.Name = std::string(m_ModalProfileName);

  m_Profiles.emplace_back(profile);
  m_SelectedProfile = m_Profiles.size() - 1;
  m_Camera.Profile = profile;

  g_mainHandle->OnConfigChanged();
}

void CameraManager::SaveProfiles()
{
  for (auto& profile : m_Profiles)
  {
    std::string path = "./Cinematic Tools/Profiles/" + profile.Name + ".ini";
    std::fstream file;

    file.open(path.c_str(), std::ios_base::out | std::ios_base::trunc);
    if (!file.is_open())
    {
      util::log::Error("Could not open file to save profile %s", path.c_str());
      continue;
    }

    file << "[CameraProfile]" << std::endl;
    file << "Name = " << profile.Name << std::endl;
    file << "FieldOfView = " << std::to_string(profile.FieldOfView) << std::endl;
    file << "MovementSpeed = " << std::to_string(profile.MovementSpeed) << std::endl;
    file << "RotationSpeed = " << std::to_string(profile.RotationSpeed) << std::endl;
    file << "RollSpeed = " << std::to_string(profile.RollSpeed) << std::endl;
    file << "FovSpeed = " << std::to_string(profile.FovSpeed) << std::endl;
    file << "DofScale = " << std::to_string(profile.DofScale) << std::endl;
    file << "DofStrength = " << std::to_string(profile.DofStrength) << std::endl;
    file << "FocusDistance = " << std::to_string(profile.FocusDistance) << std::endl;
    file << "IsProfile = true" << std::endl;

    file.close();
  }
}

void CameraManager::ToggleHUD()
{
  m_HideUI = !m_HideUI;
  util::log::Write("Hide UI: %s", m_HideUI ? "True" : "False");
  CATHODE::Scaleform* pScaleform = CATHODE::Scaleform::Singleton();

  for (unsigned int i = 0; i < pScaleform->m_ObjectCount; ++i)
  {
    CATHODE::ScaleformObject* pObject = pScaleform->m_ScaleformObjects[i];
    if (pObject)
    {
      std::string name(pObject->m_Name);
      if (name == "ingameui" || name == "weaponstuff")
        pObject->m_ForceHide = m_HideUI;
    }
  }
}