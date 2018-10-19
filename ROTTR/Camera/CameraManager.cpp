#include "CameraManager.h"
#include "../Globals.h"
#include "../Util/Util.h"
#include "../Util/ImGuiEXT.h"

using namespace DirectX;

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
  m_HudDisabled(false),
  m_TimeFreezeEnabled(false)
{
  util::log::Ok("Camera manager initialized");
}

CameraManager::~CameraManager()
{

}

void CameraManager::HotkeyUpdate()
{
  InputSystem* pInput = g_mainHandle->GetInputSystem();

  if (pInput->WentDown(Action::ToggleCamera))
    ToggleCamera();

  if (pInput->WentDown(Action::ToggleHUD))
  {
    m_HudDisabled = !m_HudDisabled;
  }

  if (pInput->WentDown(Action::ToggleFreezeTime))
  {
    m_TimeFreezeEnabled = !m_TimeFreezeEnabled;

    if (m_TimeFreezeEnabled)
    {
      typedef void(__fastcall* tFreezeGame)(int a1);
      tFreezeGame FreezeGame = (tFreezeGame)util::offsets::GetOffset("OFFSET_FREEZEGAME");
      FreezeGame(8);
    }
    else
    {
      typedef void(__fastcall* tUnfreezeGame)(int a1);
      tUnfreezeGame UnfreezeGame = (tUnfreezeGame)util::offsets::GetOffset("OFFSET_UNFREEZEGAME");
      UnfreezeGame(8);
    }
  }

  if (m_CameraEnabled)
  {
    if (pInput->WentDown(Action::Track_CreateNode))
    {
      m_TrackPlayer.CreateNode(m_Camera);
    }

    if (pInput->WentDown(Action::Track_DeleteNode))
    {
      m_TrackPlayer.DeleteNode();
    }

    if (pInput->WentDown(Action::Track_Play))
    {
      m_TrackPlayer.Toggle();
    }
  }
}

void CameraManager::OnCameraUpdate(Foundation::GameRender* pGameRender)
{
  boost::chrono::duration<double> dt = boost::chrono::high_resolution_clock::now() - m_dtCameraUpdate;
  m_dtCameraUpdate = boost::chrono::high_resolution_clock::now();
  if (!m_CameraEnabled) return;

  UpdateCamera(dt.count());

  // Switch Y and Z components before overriding
  XMMATRIX cameraTransform = XMLoadFloat4x4(&m_Camera.Transform);
  for (int i = 0; i < 4; ++i)
  {
    float val = cameraTransform.r[i].m128_f32[1];
    cameraTransform.r[i].m128_f32[1] = cameraTransform.r[i].m128_f32[2];
    cameraTransform.r[i].m128_f32[2] = val;
  }

  pGameRender->m_FieldOfView = XMConvertToRadians(m_Camera.FieldOfView);
  pGameRender->m_CameraTransform = cameraTransform;
  pGameRender->m_PrevTransform = cameraTransform;
}

void CameraManager::Update(double dt)
{
  if (!m_CameraEnabled) return;
  if (m_UIRequestReset) ResetCamera();

  UpdateInput(dt);
}

void CameraManager::DrawUI()
{
  ImGuiIO& io = ImGui::GetIO();

  ImGui::Dummy(ImVec2(0, 10));
  ImGui::Dummy(ImVec2(300, 0));

  ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 1, 1, 1));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
  ImGui::PushFont(io.Fonts->Fonts[3]);
  ImGui::SameLine();
  if (ImGui::ToggleButton(m_CameraEnabled ? "Disable" : "Enable", ImVec2(100, 25), m_CameraEnabled, true))
    ToggleCamera();
  ImGui::SameLine();
  m_UIRequestReset |= ImGui::Button("Reset", ImVec2(100, 25));

  ImGui::PopFont();
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();
  ImGui::PushFont(io.Fonts->Fonts[4]);

  ImGui::Dummy(ImVec2(0, 0));
  ImGui::Columns(4, 0, false);
  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 12);

  ImGui::PushItemWidth(200);
  bool configChanged = false;

  ImGui::Text("Movement speed");
  configChanged |= ImGui::InputFloat("##CameraMovementSpeed", &m_Camera.MovementSpeed, 0.1f, 1.0f, 2);
  ImGui::Text("Rotation speed");
  configChanged |= ImGui::InputFloat("##CameraRotationSpeed", &m_Camera.RotationSpeed, 0.1f, 1.0f, 2);
  ImGui::Text("Roll speed");
  configChanged |= ImGui::InputFloat("##CameraRollSpeed", &m_Camera.RollSpeed, 0.1f, 1.0f, 2);
  ImGui::Text("FoV speed");
  configChanged |= ImGui::InputFloat("##CameraFoVSpeed", &m_Camera.FovSpeed, 0.1f, 1.0f, 2);

  if (configChanged)
    g_mainHandle->OnConfigChanged();

  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 290);
  ImGui::PushItemWidth(200);

  ImGui::Text("Field of view");
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 10));
  ImGui::InputFloat("##CameraFoV", &m_Camera.FieldOfView, 1.f, 1.f, 2);
  ImGui::Checkbox("Reset camera automatically", &m_AutoReset);
  ImGui::Checkbox("Smooth mouse", &m_SmoothMouse);
  ImGui::Checkbox("Disable player KBM input", &m_KbmDisabled);
  ImGui::Checkbox("Disable player gamepad input", &m_GamepadDisabled);
  ImGui::PopStyleVar();

  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 552);
  ImGui::PushItemWidth(200);

  m_TrackPlayer.DrawUI();
  ImGui::PopFont();
}

void CameraManager::ReadConfig(INIReader* pReader)
{
  m_Camera.MovementSpeed = pReader->GetReal("Camera", "MovementSpeed", 1.0f);
  m_Camera.RotationSpeed = pReader->GetReal("Camera", "RotationSpeed", XM_PI / 4);
  m_Camera.RollSpeed = pReader->GetReal("Camera", "RollSpeed", XM_PI / 8);
  m_Camera.FovSpeed = pReader->GetReal("Camera", "FovSpeed", 5.0f);
  m_AutoReset = pReader->GetBoolean("Camera", "AutoReset", false);
}

const std::string CameraManager::GetConfig() const
{
  std::string config = "[Camera]\n";
  config += "MovementSpeed = " + std::to_string(m_Camera.MovementSpeed) + "\n";
  config += "RotationSpeed = " + std::to_string(m_Camera.RotationSpeed) + "\n";
  config += "RollSpeed = " + std::to_string(m_Camera.RollSpeed) + "\n";
  config += "FovSpeed = " + std::to_string(m_Camera.FovSpeed) + "\n";
  config += "AutoReset = " + std::to_string(m_AutoReset) + "\n";

  return config;
}

void CameraManager::UpdateCamera(double dt)
{
  XMVECTOR qPitch = XMQuaternionRotationRollPitchYaw(m_CameraInput.dPitch * dt * m_Camera.RotationSpeed, 0, 0);
  XMVECTOR qYaw = XMQuaternionRotationRollPitchYaw(0, m_CameraInput.dYaw* dt * m_Camera.RotationSpeed, 0);
  XMVECTOR qRoll = XMQuaternionRotationRollPitchYaw(0, 0, m_CameraInput.dRoll* dt * m_Camera.RollSpeed);
  
  XMVECTOR qRotation = XMLoadFloat4(&m_Camera.Rotation);
  XMVECTOR vPosition = XMLoadFloat3(&m_Camera.Position);

  // Add delta rotations to camera rotation
  qRotation = XMQuaternionMultiply(qPitch, qRotation);
  qRotation = XMQuaternionMultiply(qRotation, qYaw);
  qRotation = XMQuaternionMultiply(qRoll, qRotation);

  // Make sure it's normalized
  qRotation = XMQuaternionNormalize(qRotation);

  m_Camera.FieldOfView += m_CameraInput.dFocalLength * dt * m_Camera.FovSpeed;

  // If a camera track is being played, get the current
  // state and overwrite position/rotation/FoV.
  if (m_TrackPlayer.IsPlaying())
  {
    CatmullRomNode resultNode = m_TrackPlayer.PlayForward(dt);

    if (m_TrackPlayer.IsRotationLocked())
      qRotation = XMLoadFloat4(&resultNode.Rotation);

    if (m_TrackPlayer.IsFovLocked())
      m_Camera.FieldOfView = resultNode.FieldOfView;

    vPosition = XMLoadFloat3(&resultNode.Position);
  }

  // Create rotation matrix
  XMMATRIX rotMatrix = XMMatrixRotationQuaternion(qRotation);

  // Add delta positions
  // * 100 because ROTTR coordinates seem to be in centimeters
  if (!m_TrackPlayer.IsPlaying())
  {
    vPosition += m_CameraInput.dX * rotMatrix.r[0] * dt * m_Camera.MovementSpeed * 100;
    vPosition += m_CameraInput.dY * rotMatrix.r[1] * dt * m_Camera.MovementSpeed * 100;
    vPosition += m_CameraInput.dZ * rotMatrix.r[2] * dt * m_Camera.MovementSpeed * 100;
  }

  // Store results
  rotMatrix.r[3] = vPosition;
  rotMatrix.r[3].m128_f32[3] = 1.0f;

  XMStoreFloat3(&m_Camera.Position, vPosition);
  XMStoreFloat4(&m_Camera.Rotation, qRotation);
  XMStoreFloat4x4(&m_Camera.Transform, rotMatrix);

  m_CameraInput.Clear();
}

void CameraManager::UpdateInput(double dt)
{
  InputSystem* pInput = g_mainHandle->GetInputSystem();
  if (!g_hasFocus || g_mainHandle->GetUI()->HasKeyboardFocus())
    return;

  // These need to be changed according to the right/left-handness of game camera
  if (m_KbmDisabled)
  {
    m_CameraInput.dX = pInput->GetActionState(Camera_Right) - pInput->GetActionState(Camera_Left);
    m_CameraInput.dY = pInput->GetActionState(Camera_Up) - pInput->GetActionState(Camera_Down);
    m_CameraInput.dZ = pInput->GetActionState(Camera_Forward) - pInput->GetActionState(Camera_Backward);
  }
  else
  {
    m_CameraInput.dX = pInput->GetActionState(Camera_RightSecondary) - pInput->GetActionState(Camera_LeftSecondary);
    m_CameraInput.dY = pInput->GetActionState(Camera_UpSecondary) - pInput->GetActionState(Camera_DownSecondary);
    m_CameraInput.dZ = pInput->GetActionState(Camera_ForwardSecondary) - pInput->GetActionState(Camera_BackwardSecondary);
  }

  m_CameraInput.dPitch = pInput->GetActionState(Camera_PitchDown) - pInput->GetActionState(Camera_PitchUp);
  m_CameraInput.dYaw = pInput->GetActionState(Camera_YawRight) - pInput->GetActionState(Camera_YawLeft);
  m_CameraInput.dRoll = pInput->GetActionState(Camera_RollLeft) - pInput->GetActionState(Camera_RollRight);
  m_CameraInput.dFocalLength = pInput->GetActionState(Camera_IncFieldOfView) - pInput->GetActionState(Camera_DecFieldOfView);
  if (m_KbmDisabled && !g_mainHandle->GetUI()->IsEnabled())
  {
    XMFLOAT3 state = pInput->GetMouseState();
    float sensitivity = pInput->GetMouseSensitivity();

    m_MouseBuffer.AddValue(state);
    XMFLOAT3 smoothState = m_MouseBuffer.CalcAverage();

    if (m_SmoothMouse)
      state = smoothState;

    m_CameraInput.dPitch += state.y * sensitivity;
    m_CameraInput.dYaw += state.x * sensitivity;
    m_CameraInput.dFocalLength += smoothState.z;
  }
}

void CameraManager::ToggleCamera()
{
  // If first enable, fetch game camera location
  if (m_FirstEnable || m_AutoReset)
  {
    XMVECTOR cameraPos = Foundation::GameRender::Singleton()->m_CameraTransform.r[3];
    m_Camera.Position = XMFLOAT3(cameraPos.m128_f32[0], cameraPos.m128_f32[2], cameraPos.m128_f32[1]);
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