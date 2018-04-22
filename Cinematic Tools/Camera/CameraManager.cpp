#include "CameraManager.h"
#include "../Main.h"
#include "../Util/ImGuiEXT.h"
#include <Windows.h>

using namespace DirectX;

CameraManager::CameraManager() :
  m_CameraEnabled(false),
  m_FirstEnable(true),
  m_GamepadDisabled(true),
  m_KbmDisabled(true),
  m_Camera(),
  m_TrackPlayer()
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
      Sleep(1);
  }

  if (pInput->IsActionDown(Action::ToggleHUD))
  {

    while (pInput->IsActionDown(Action::ToggleHUD))
      Sleep(1);
  }
}

void CameraManager::Update(double dt)
{
  if (!m_CameraEnabled) return;

  UpdateInput(dt);
  UpdateCamera(dt);
}

void CameraManager::DrawUI()
{
  ImGuiIO& io = ImGui::GetIO();

  ImGui::Dummy(ImVec2(0, 10));
  ImGui::Dummy(ImVec2(300, 0));
}

void CameraManager::ReadConfig(INIReader* pReader)
{
  m_Camera.MovementSpeed = pReader->GetReal("Camera", "MovementSpeed", 1.0f);
  m_Camera.RotationSpeed = pReader->GetReal("Camera", "RotationSpeed", XM_PI / 4);
  m_Camera.RollSpeed = pReader->GetReal("Camera", "RollSpeed", XM_PI / 8);
  m_Camera.FovSpeed = pReader->GetReal("Camera", "FovSpeed", 5.0f);
}

const std::string CameraManager::GetConfig()
{
  std::string config = "[Camera]\n";
  config += "MovementSpeed = " + std::to_string(m_Camera.MovementSpeed) + "\n";
  config += "RotationSpeed = " + std::to_string(m_Camera.RotationSpeed) + "\n";
  config += "RollSpeed = " + std::to_string(m_Camera.RollSpeed) + "\n";
  config += "FovSpeed = " + std::to_string(m_Camera.FovSpeed) + "\n";

  return config;
}

void CameraManager::UpdateCamera(double dt)
{
  XMVECTOR qPitch = XMQuaternionRotationRollPitchYaw(m_Camera.dPitch, 0, 0);
  XMVECTOR qYaw = XMQuaternionRotationRollPitchYaw(0, m_Camera.dYaw, 0);
  XMVECTOR qRoll = XMQuaternionRotationRollPitchYaw(0, 0, m_Camera.dRoll);
  
  XMVECTOR qRotation = XMLoadFloat4(&m_Camera.Rotation);
  XMVECTOR vPosition = XMLoadFloat3(&m_Camera.Position);

  // Adds delta rotations to camera rotation
  qRotation = XMQuaternionMultiply(qPitch, qRotation);
  qRotation = XMQuaternionMultiply(qRotation, qYaw);
  qRotation = XMQuaternionMultiply(qRoll, qRotation);

  // Make sure it's normalized
  qRotation = XMQuaternionNormalize(qRotation);
  m_Camera.FieldOfView += m_Camera.dFov;

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
  if (!m_TrackPlayer.IsPlaying())
  {
    vPosition += dt * m_Camera.dX * rotMatrix.r[0];
    vPosition += dt * m_Camera.dY * rotMatrix.r[2];
    vPosition += dt * m_Camera.dZ * rotMatrix.r[1];
  }

  // Store results
  rotMatrix.r[3] = vPosition;
  rotMatrix.r[3].m128_f32[3] = 1.0f;

  XMStoreFloat3(&m_Camera.Position, vPosition);
  XMStoreFloat4(&m_Camera.Rotation, qRotation);
  XMStoreFloat4x4(&m_Camera.Transform, rotMatrix);
}

void CameraManager::UpdateInput(double dt)
{
  InputSystem* pInput = g_mainHandle->GetInputSystem();

  m_Camera.dX = pInput->GetActionState(Camera_Left)     - pInput->GetActionState(Camera_Right);
  m_Camera.dY = pInput->GetActionState(Camera_Up)       - pInput->GetActionState(Camera_Down);
  m_Camera.dZ = pInput->GetActionState(Camera_Forward)  - pInput->GetActionState(Camera_Backward);

  m_Camera.dPitch = pInput->GetActionState(Camera_PitchUp)  - pInput->GetActionState(Camera_PitchDown);
  m_Camera.dYaw   = pInput->GetActionState(Camera_YawLeft)  - pInput->GetActionState(Camera_YawRight);
  m_Camera.dRoll  = pInput->GetActionState(Camera_RollLeft) - pInput->GetActionState(Camera_RollRight);
  m_Camera.dFov   = pInput->GetActionState(Camera_IncFov)   - pInput->GetActionState(Camera_DecFov);

  m_Camera.dX *= dt * m_Camera.MovementSpeed;
  m_Camera.dY *= dt * m_Camera.MovementSpeed;
  m_Camera.dZ *= dt * m_Camera.MovementSpeed;

  m_Camera.dPitch *= dt * m_Camera.RotationSpeed;
  m_Camera.dYaw   *= dt * m_Camera.RotationSpeed;
  m_Camera.dRoll  *= dt * m_Camera.RotationSpeed;
  m_Camera.dFov   *= dt * m_Camera.FovSpeed;

  // TODO: Handle mouse input
}

void CameraManager::ToggleCamera()
{
  // If first enable, fetch game camera location
  if (m_FirstEnable)
  {
    // Get game camera, set m_Camera.position
    m_FirstEnable = false;
  }

  m_CameraEnabled = !m_CameraEnabled;
}

void CameraManager::ResetCamera()
{
  // There's a few ways you can do this
  // Best way might depend on the game

  // In BF tools camera is toggled off, then Sleep() for a few
  // 100 ms, fetch the game camera location again and reset to
  // that and then re-enable camera again.

  if (!m_CameraEnabled) return;
  m_CameraEnabled = false;
  Sleep(100);

  // m_Camera.Position = pGameCameraSomewhere->m_position or w/e
  m_Camera.Rotation = XMFLOAT4(0, 0, 0, 1);

  m_CameraEnabled = true;
}