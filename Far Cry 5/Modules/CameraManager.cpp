#include "CameraManager.h"
#include "../Main.h"
#include "../Util/Util.h"
#include "../Util/ImGuiHelpers.h"

// How often should the mouse buffer be updated (in seconds)
static const float g_mouseBufferUpdateFreq = 0.005f;
static __int64 g_cameraActivatorVTable = 0x0;

using namespace DirectX;

CameraManager::CameraManager() :
  m_CameraEnabled(false),
  m_FirstEnable(true),
  m_GamepadDisabled(true),
  m_Camera(Camera()),
  m_pActivator(nullptr),
  m_uiRequestReset(false),
  m_uiRequestToggle(false),
  m_GameUIDisabled(false),
  m_dtBufferUpdate(0)
{
  ZeroMemory(m_mousePitchBuffer, sizeof(m_mousePitchBuffer));
  ZeroMemory(m_mouseYawBuffer, sizeof(m_mouseYawBuffer));

  INIReader* pReader = g_mainHandle->GetConfig()->GetReader();
  m_Camera.movementSpeed = pReader->GetReal("Camera", "MovementSpeed", 1.0f);
  m_Camera.rotationSpeed = pReader->GetReal("Camera", "RotationSpeed", XM_PI / 4);
  m_Camera.rollSpeed = pReader->GetReal("Camera", "RollSpeed", XM_PI / 8);
  m_Camera.fovSpeed = pReader->GetReal("Camera", "FovSpeed", 2.0f);

  g_cameraActivatorVTable = (__int64)FC::FCHandle + 0x4297368;
}

CameraManager::~CameraManager()
{

}

bool print = true;

void CameraManager::CameraHook(FC::CMarketingCamera* pGameCamera)
{
  if (print)
  {
    print = false;
    util::log::Write("CMarketingCamera 0x%I64X", pGameCamera);
  }

  if (!m_CameraEnabled) return;

  XMFLOAT4X4& gameMatrix = pGameCamera->GetTransform();
  XMFLOAT3* position = (XMFLOAT3*)&gameMatrix.m[3][0];

  if (m_FirstEnable)
  {
    m_FirstEnable = false;
    m_Camera.position = *position;
  }

  *position = m_Camera.position;
  pGameCamera->m_FieldofView = XMConvertToRadians(m_Camera.fov);
  pGameCamera->m_NearPlane = m_Camera.nearPlane;
  pGameCamera->m_FarPlane = m_Camera.farPlane;
  pGameCamera->m_DofEnable = m_Dof.enabled;
  pGameCamera->m_DofOverride = m_Dof.enabled;
  pGameCamera->m_DofFocusDistance = m_Dof.focusDistance;
  pGameCamera->m_DofNear = m_Dof.nearDistance;
  pGameCamera->m_DofFar = m_Dof.farDistance;
  pGameCamera->m_DofCoC = m_Dof.cocSize;
}

void __fastcall CameraManager::ComponentHook(FC::ComponentCollection<__int64>* pCollection)
{
  if (m_pActivator != nullptr)
  {
    __int64 vtable = *(__int64*)((__int64)m_pActivator-0x20);
    if (vtable != g_cameraActivatorVTable)
    {
      util::log::Write("ActivatorComponent was destroyed");
      m_pActivator = nullptr;
      return;
    }

    if (m_uiRequestToggle)
    {
      m_uiRequestToggle = false;
      ToggleCamera();
    }
    if (m_uiRequestReset)
    {
      m_uiRequestReset = false;
      ResetCamera();
    }

    return;
  }

  for (unsigned i = 0; i < pCollection->size; ++i)
  {
    __int64 pComponent = pCollection->pComponents[i];
    if (!pComponent) continue;

    __int64 vtable = *(__int64*)(pComponent);
    if (vtable == g_cameraActivatorVTable)
    {
      m_pActivator = (FC::CMarketingCameraActivator*)(pComponent+0x20);
      util::log::Write("Found ActivatorComponent 0x%I64X", m_pActivator);
      break;
    }
  }
}

void CameraManager::AngleHook(__int64 pMatrix)
{
  if (!m_CameraEnabled) return;
  XMFLOAT4X4* rotMatrix = reinterpret_cast<XMFLOAT4X4*>(pMatrix);
  *rotMatrix = m_Camera.rotMatrix;
}

void CameraManager::HotkeyUpdate()
{
  InputManager* pInput = g_mainHandle->GetInputManager();
  if (pInput->IsActionDown(Camera_Toggle))
  {
    m_uiRequestToggle = true;

    while (pInput->IsActionDown(Camera_Toggle))
      Sleep(10);
  }

  if (pInput->IsActionDown(Camera_HideHUD))
  {
    m_GameUIDisabled = !m_GameUIDisabled;
    while (pInput->IsActionDown(Camera_HideHUD))
      Sleep(10);
  }

  if (pInput->IsActionDown(Camera_FreezeTime))
  {
    FC::CTimer* pTimer = FC::CTimer::Singleton();
    pTimer->m_FreezeTime = !pTimer->m_FreezeTime;

    while (pInput->IsActionDown(Camera_FreezeTime))
      Sleep(10);
  }

  if (m_CameraEnabled)
  {
    if (pInput->IsActionDown(Track_CreateKey))
    {
      m_TrackManager.CreateNode(m_Camera);

      while (pInput->IsActionDown(Track_CreateKey))
        Sleep(10);
    }
    if (pInput->IsActionDown(Track_DeleteKey))
    {
      m_TrackManager.DeleteNode();

      while (pInput->IsActionDown(Track_DeleteKey))
        Sleep(10);
    }
    if (pInput->IsActionDown(Track_Play))
    {
      m_TrackManager.Play();

      while (pInput->IsActionDown(Track_Play))
        Sleep(10);
    }
  }
}

void CameraManager::Update(double dt)
{
  if (!m_CameraEnabled) return;
  UpdateInput(dt);
  UpdateCamera(dt);
}

void CameraManager::UpdateCamera(double dt)
{
  XMVECTOR qPitch = XMQuaternionRotationRollPitchYaw(-m_Camera.dPitch * dt * m_Camera.rotationSpeed, 0, 0);
  XMVECTOR qYaw = XMQuaternionRotationRollPitchYaw(0, -m_Camera.dYaw * dt * m_Camera.rotationSpeed, 0);
  XMVECTOR qRoll = XMQuaternionRotationRollPitchYaw(0, 0, m_Camera.dRoll * dt * m_Camera.rollSpeed);
  XMVECTOR qRotation = XMLoadFloat4(&m_Camera.rotation);
  XMVECTOR vPosition = XMLoadFloat3(&m_Camera.position);

  qRotation = XMQuaternionMultiply(qPitch, qRotation);
  qRotation = XMQuaternionMultiply(qRotation, qYaw);
  qRotation = XMQuaternionMultiply(qRoll, qRotation);

  m_Camera.fov += dt * m_Camera.dFov * m_Camera.fovSpeed;

  if (m_TrackManager.IsPlaying())
  {
    CatmullRomNode result = m_TrackManager.PlayForward(dt);
    if (m_TrackManager.IsRotationLocked())
      qRotation = XMLoadFloat4(&result.qRotation);
    if (m_TrackManager.IsFovLocked())
      m_Camera.fov = result.fov;
    vPosition = XMLoadFloat3(&result.vPosition);
  }

  XMMATRIX rotMatrix = XMMatrixRotationQuaternion(qRotation);
  for (int i = 0; i < 3; i++)
  {
    float z = rotMatrix.r[i].m128_f32[1];
    rotMatrix.r[i].m128_f32[1] = rotMatrix.r[i].m128_f32[2];
    rotMatrix.r[i].m128_f32[2] = z;
  }
  XMVECTOR up = rotMatrix.r[1];
  rotMatrix.r[1] = rotMatrix.r[2];
  rotMatrix.r[2] = up;
  rotMatrix.r[3] = XMVectorSet(0, 0, 0, 1);

  if (!m_TrackManager.IsPlaying())
  {
    vPosition += dt * m_Camera.movementSpeed * m_Camera.dX * rotMatrix.r[0];
    vPosition += dt * m_Camera.movementSpeed * m_Camera.dY * rotMatrix.r[2];
    vPosition += dt * m_Camera.movementSpeed * m_Camera.dZ * rotMatrix.r[1];
  }

  XMStoreFloat3(&m_Camera.position, vPosition);
  XMStoreFloat4(&m_Camera.rotation, qRotation);
  XMStoreFloat4x4(&m_Camera.rotMatrix, rotMatrix);
}

void CameraManager::UpdateInput(double dt)
{
  m_Camera.dY = m_Camera.dZ = m_Camera.dX = 0;
  m_Camera.dPitch = m_Camera.dYaw = m_Camera.dRoll = 0;
  m_Camera.dFov = 0;

  if (!FC::IsFocused()) return;

  InputManager* pInput = g_mainHandle->GetInputManager();

  m_Camera.dX += pInput->GetActionState(Camera_Right) - pInput->GetActionState(Camera_Left);
  m_Camera.dY += pInput->GetActionState(Camera_Up) - pInput->GetActionState(Camera_Down);
  m_Camera.dZ += pInput->GetActionState(Camera_Forward) - pInput->GetActionState(Camera_Backward);

  m_Camera.dYaw += pInput->GetActionState(Camera_YawLeft) - pInput->GetActionState(Camera_YawRight);
  m_Camera.dPitch += pInput->GetActionState(Camera_PitchUp) - pInput->GetActionState(Camera_PitchDown);
  m_Camera.dRoll += pInput->GetActionState(Camera_RollLeft) - pInput->GetActionState(Camera_RollRight);

  m_Camera.dFov += pInput->GetActionState(Camera_IncFov) - pInput->GetActionState(Camera_DecFov);

  for (unsigned i = 0; i < 50; ++i)
  {
    float indexMultiplier = (50 - i) / 50.f;
    m_Camera.dYaw += indexMultiplier * m_mouseYawBuffer[i] / 50;
    m_Camera.dPitch += indexMultiplier * m_mousePitchBuffer[i] / 50;
  }

  m_dtBufferUpdate += dt;
  if (m_dtBufferUpdate < g_mouseBufferUpdateFreq)
    return;

  m_dtBufferUpdate = 0;
  for (unsigned i = 49; i > 0; i -= 1)
  {
    m_mousePitchBuffer[i] = m_mousePitchBuffer[i - 1];
    m_mouseYawBuffer[i] = m_mouseYawBuffer[i - 1];
  }

  m_mouseYawBuffer[0] = 0;
  m_mousePitchBuffer[0] = 0;

  if (!g_mainHandle->GetUI()->IsEnabled() && FC::IsMouseConfined())
  {
    int mouseX = 0, mouseY = 0;
    std::tie(mouseX, mouseY) = pInput->GetMouseState();
    m_mouseYawBuffer[0] = -mouseX * 0.1f;
    m_mousePitchBuffer[0] = -mouseY * 0.1f;
  }
}

void CameraManager::ToggleCamera()
{
  if (m_pActivator == nullptr)
  {
    util::log::Error("CMarketingCameraActivator is null");
    return;
  }

  m_pActivator->ToggleCamera();
  m_CameraEnabled = m_pActivator->m_CameraEnabled;
}

void CameraManager::ResetCamera()
{
  if (!m_CameraEnabled) return;
  m_Camera.rotation = XMFLOAT4(0, 0, 0, 1);
  m_FirstEnable = true;
  ToggleCamera();
  Sleep(100);
  ToggleCamera();
}

void CameraManager::DrawUI()
{
  ImGuiIO& io = ImGui::GetIO();

  ImGui::Dummy(ImVec2(0, 10));
  ImGui::Dummy(ImVec2(300, 0));

  //ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 1, 1, 1));
  //ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
  ImGui::PushFont(io.Fonts->Fonts[3]);
  ImGui::SameLine();
  ImGui::DrawWithBorders([=]
  {
    m_uiRequestToggle |= ImGui::ToggleButton(m_CameraEnabled ? "Disable" : "Enable", ImVec2(100, 25), m_CameraEnabled, true);
    ImGui::SameLine();
    m_uiRequestReset |= ImGui::Button("Reset", ImVec2(100, 25));
  });

  ImGui::PopFont();
  //ImGui::PopStyleColor();
  //ImGui::PopStyleVar();
  ImGui::PushFont(io.Fonts->Fonts[4]);

  ImGui::Dummy(ImVec2(0, 0));
  ImGui::Columns(4, 0, false);
  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 12);

  bool markDirty = false;

  ImGui::PushItemWidth(200);
  ImGui::Text("Movement Speed");
  markDirty |= ImGui::InputFloat("##CameraMoveSpeed", &m_Camera.movementSpeed, 1, 5, 2);
  ImGui::Text("Rotation Speed");
  markDirty |= ImGui::InputFloat("##CameraRotSpeed", &m_Camera.rotationSpeed, 0.1, 0.2, 2);
  ImGui::Text("Roll Speed");
  markDirty |= ImGui::InputFloat("##CameraRollSpeed", &m_Camera.rollSpeed, 0.1, 0.2, 2);
  ImGui::Text("FOV Speed");
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 10));
  markDirty |= ImGui::InputFloat("##CameraFovSpeed", &m_Camera.fovSpeed, 0.5, 0.2, 2);
  //ImGui::Checkbox("Disable keyboard & mouse input", &m_kbmDisabled);
  ImGui::Checkbox("Disable gamepad input", &m_GamepadDisabled);

  ImGui::PopStyleVar();
  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 290);
  ImGui::PushItemWidth(200);
  ImGui::Text("Field of view");
  ImGui::InputFloat("##CameraFoV", &m_Camera.fov, 1, 5, 2);
  ImGui::Text("Near distance");
  ImGui::InputFloat("##CameraNear", &m_Camera.nearPlane, 0.1, 1.0, 2);
  ImGui::Text("Far distance");
  ImGui::InputFloat("##CameraFar", &m_Camera.farPlane, 0.1, 1.0, 2);

  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 552);
  ImGui::PushItemWidth(200);

  m_TrackManager.DrawUI();

  ImGui::PopFont();

  if (markDirty)
    g_mainHandle->GetConfig()->MarkDirty();
}

const std::string CameraManager::GetConfig()
{
  std::string config = "[Camera]\n";
  config += "MovementSpeed = " + std::to_string(m_Camera.movementSpeed) + "\n";
  config += "RotationSpeed = " + std::to_string(m_Camera.movementSpeed) + "\n";
  config += "RollSpeed = " + std::to_string(m_Camera.movementSpeed) + "\n";
  config += "FovSpeed = " + std::to_string(m_Camera.fovSpeed) + "\n";

  return config;
}