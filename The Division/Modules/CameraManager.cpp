#include "CameraManager.h"
#include "Snowdrop.h"
#include "../Main.h"
#include "../Util/Util.h"
#include "../imgui/imgui.h"
#include "../Util/ImGuiHelpers.h"
#include <stdio.h>

CameraManager::CameraManager()
{
  m_cameraEnabled = false;
  m_firstEnable = true;
  m_camera.pitch = 0;
  m_camera.yaw = 0;
  m_camera.roll = 0;

  m_camera.fov = 40.f;

  CameraTrack defaultTrack;
  defaultTrack.name = "Track #1";
  defaultTrack.nodes.clear();
  m_runningId = 2;

  m_tracks.push_back(defaultTrack);
  m_trackNameList = new const char*[1];
  m_trackNameList[0] = (const char*)m_tracks[0].name.c_str();
  m_selectedTrackIndex = 0;

  m_lockToPlayer = false;
  m_selectedPlayerIndex = 0;
  UpdatePlayerList();

  m_shakeInfo.qRotations[0] = m_shakeInfo.qRotations[1] = XMQuaternionIdentity();

  for (int i = 2; i < 4; ++i)
  {
    float randPitch = fmod(rand(), m_shakeInfo.maxAngle * 2) - m_shakeInfo.maxAngle;
    float randYaw = fmod(rand(), m_shakeInfo.maxAngle * 2) - m_shakeInfo.maxAngle;
    m_shakeInfo.qRotations[i] = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(randPitch), XMConvertToRadians(randYaw), 0);
  }
}

CameraManager::~CameraManager()
{
}

void CameraManager::Update(double dt)
{
  UpdatePlayerList();
  if (!m_cameraEnabled) return;
  if (m_shakeInfo.shakeEnabled) GenerateShake(dt);

  if (m_trackState.playing) PlayTrackForward(dt);
  InputManager* pInputManager = g_mainHandle->GetInputManager();

  if (pInputManager->IsKeyDown(InputManager::Action::Track_CreateNode))
  {
    CreateNode();
    while (pInputManager->IsKeyDown(InputManager::Action::Track_CreateNode))
      Sleep(1);
  }

  if (pInputManager->IsKeyDown(InputManager::Action::Track_DeleteNode))
  {
    DeleteNode();
    while (pInputManager->IsKeyDown(InputManager::Action::Track_DeleteNode))
      Sleep(1);
  }

  if (pInputManager->IsKeyDown(InputManager::Action::Track_Play))
  {
    ToggleTrackPlay();
    while (pInputManager->IsKeyDown(InputManager::Action::Track_Play))
      Sleep(1);
  }

  if (!m_trackState.playing || !m_trackState.rotationLocked)
  {
    m_camera.yaw += m_settings.rotationSpeed * dt * (pInputManager->GetActionState(InputManager::Action::Camera_YawRight) - pInputManager->GetActionState(InputManager::Action::Camera_YawLeft));
    m_camera.pitch += m_settings.rotationSpeed * dt * (pInputManager->GetActionState(InputManager::Action::Camera_PitchDown) - pInputManager->GetActionState(InputManager::Action::Camera_PitchUp));
    m_camera.roll += m_settings.rollSpeed * dt * (pInputManager->GetActionState(InputManager::Action::Camera_RollLeft) - pInputManager->GetActionState(InputManager::Action::Camera_RollRight));
  }

  XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_camera.pitch), XMConvertToRadians(m_camera.yaw), 0);
  XMMATRIX rollMatrix = XMMatrixRotationRollPitchYaw(0, 0, XMConvertToRadians(m_camera.roll));
  rotationMatrix = XMMatrixMultiply(rollMatrix, rotationMatrix);

  if (!m_trackState.playing)
  {
    float dX = 0, dY = 0, dZ = 0;

    dX += pInputManager->GetActionState(InputManager::Action::Camera_Right);
    dX -= pInputManager->GetActionState(InputManager::Action::Camera_Left);

    dY += pInputManager->GetActionState(InputManager::Action::Camera_Up);
    dY -= pInputManager->GetActionState(InputManager::Action::Camera_Down);

    dZ += pInputManager->GetActionState(InputManager::Action::Camera_Forward);
    dZ -= pInputManager->GetActionState(InputManager::Action::Camera_Backward);

    m_camera.position += dX * dt * m_settings.movementSpeed * rotationMatrix.r[0];
    m_camera.position += dY * dt * m_settings.movementSpeed * rotationMatrix.r[1];
    m_camera.position += dZ * dt * m_settings.movementSpeed * rotationMatrix.r[2];
  }

  if (!m_trackState.playing || !m_trackState.fovLocked)
  {
    m_camera.fov += dt * m_settings.zoomSpeed * pInputManager->GetActionState(InputManager::Action::Camera_IncFov);
    m_camera.fov -= dt * m_settings.zoomSpeed * pInputManager->GetActionState(InputManager::Action::Camera_DecFov);
  }
}

void CameraManager::CameraHook(__int64 pCamera)
{
  if (!m_cameraEnabled)
    return;

  TD::GameCamera* pGameCamera = (TD::GameCamera*)pCamera;

  XMMATRIX targetMatrix = XMMatrixIdentity();
  if (m_lockToPlayer)
    targetMatrix = m_pAgents[m_selectedPlayerIndex]->m_Transform;

  XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_camera.pitch), XMConvertToRadians(m_camera.yaw), 0);
  XMMATRIX rollMatrix = XMMatrixRotationRollPitchYaw(0, 0, XMConvertToRadians(m_camera.roll));
  rotationMatrix = XMMatrixMultiply(rollMatrix, rotationMatrix);

  XMVECTOR cameraPos = targetMatrix.r[3];
  cameraPos += m_camera.position.m128_f32[0] * targetMatrix.r[0];
  cameraPos += m_camera.position.m128_f32[1] * targetMatrix.r[1];
  cameraPos += m_camera.position.m128_f32[2] * targetMatrix.r[2];

  targetMatrix = XMMatrixMultiply(rotationMatrix, targetMatrix);
  if (m_shakeInfo.shakeEnabled)
    targetMatrix = XMMatrixMultiply(m_shakeInfo.shakeMatrix, targetMatrix);

  targetMatrix.r[3] = cameraPos;

  if (m_trackState.playing && m_trackState.rotationLocked)
    targetMatrix = m_trackState.transform;
  else if (m_trackState.playing)
    targetMatrix.r[3] = m_trackState.transform.r[3];

  pGameCamera->m_Transform = targetMatrix;
  if (!m_trackState.playing || !m_trackState.fovLocked)
    pGameCamera->m_FieldOfView = XMConvertToRadians(m_camera.fov);
  else
    pGameCamera->m_FieldOfView = XMConvertToRadians(m_trackState.fov);
}

void CameraManager::ToggleCamera()
{
  m_cameraEnabled = !m_cameraEnabled;
  util::log::Write("Camera enabled: %s", m_cameraEnabled ? "True" : "False");

  if (m_cameraEnabled && m_firstEnable)
  {
    m_firstEnable = false;
    if(!m_lockToPlayer)
      m_camera.position = TD::RogueClient::Singleton()->m_pClient->m_pWorld->m_pCameraManager->m_pCamera2->m_Transform.r[3];
  }

  bool* pFreezePlayerInput = (bool*)(TD::RogueClient::Singleton()->m_pClient->m_pWorld->m_pInput + 0x4);
  *pFreezePlayerInput = m_cameraEnabled;
}

void CameraManager::ResetCamera()
{
  if (!m_cameraEnabled) return;

  m_camera.pitch = 0;
  m_camera.yaw = 0;
  m_camera.roll = 0;

  if (m_lockToPlayer)
  {
    m_camera.position = XMVectorSet(0, 1.0f, -3.0f, 1.0f);
    return;
  }

  TD::World* pWorld = TD::RogueClient::Singleton()->m_pClient->m_pWorld;
  if (!pWorld) return;

  if (pWorld->m_AgentArray && pWorld->m_AgentCount > 0)
  {
    TD::Agent* pLocalAgent = pWorld->m_AgentArray[0];
    m_camera.position = pLocalAgent->m_Transform.r[3];
    m_camera.position.m128_f32[1] += 1.0;
    m_camera.position.m128_f32[2] -= 3.0f;
  }
}

void CameraManager::CreateTrack()
{
  if (m_trackState.playing) return;

  CameraTrack newTrack;
  newTrack.name = "Track #" + std::to_string(m_runningId++);

  m_tracks.push_back(newTrack);

  const char** oldTrackList = m_trackNameList;
  const char** newTrackList = new const char*[m_tracks.size()];

  for (int i = 0; i<m_tracks.size(); ++i)
    newTrackList[i] = m_tracks[i].name.c_str();

  m_trackNameList = newTrackList;
  delete[] oldTrackList;

  m_selectedTrackIndex = m_tracks.size() - 1;
}

void CameraManager::DeleteTrack()
{
  if (m_trackState.playing || m_tracks.size() <= 1) return;

  m_tracks[m_selectedTrackIndex].nodes.clear();
  m_tracks.erase(m_tracks.begin() + m_selectedTrackIndex);

  if (m_selectedTrackIndex >= m_tracks.size())
    m_selectedTrackIndex -= 1;

  const char** oldTrackList = m_trackNameList;
  const char** newTrackList = new const char*[m_tracks.size()];

  for (int i = 0; i<m_tracks.size(); ++i)
    newTrackList[i] = m_tracks[i].name.c_str();

  m_trackNameList = newTrackList;
  delete[] oldTrackList;
}

void CameraManager::CreateNode()
{
  if (m_trackState.playing) return;
  std::vector<CameraNode>& rNodes = m_tracks[m_selectedTrackIndex].nodes;

  CameraNode newNode;
  newNode.vPos = m_camera.position;
  newNode.fov = m_camera.fov;

  XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_camera.pitch), XMConvertToRadians(m_camera.yaw), 0);
  XMMATRIX rollMatrix = XMMatrixRotationRollPitchYaw(0, 0, XMConvertToRadians(m_camera.roll));
  rotationMatrix = XMMatrixMultiply(rollMatrix, rotationMatrix);

  newNode.qRot = XMQuaternionRotationMatrix(rotationMatrix);
  if (rNodes.size() > 0)
    newNode.time = rNodes.back().time + 5.0f;
  else
    newNode.time = 0;

  rNodes.push_back(newNode);
  util::log::Write("Created camera track node %d", rNodes.size());
}

void CameraManager::DeleteNode()
{
  if (m_trackState.playing) return;

  std::vector<CameraNode>& rNodes = m_tracks[m_selectedTrackIndex].nodes;
  if (rNodes.size() == 0) return;

  rNodes.pop_back();
  util::log::Write("Deleted camera track node, %d nodes left", rNodes.size());
}

void CameraManager::ToggleTrackPlay()
{
  if (m_trackState.playing)
  {
    m_trackState.playing = false;
    return;
  }

  if (m_tracks[m_selectedTrackIndex].nodes.size() <= 1)
  {
    util::log::Write("Can't play a camera track with less than 2 nodes");
    return;
  }

  m_trackState.node = 0;
  m_trackState.time = 0;
  m_trackState.playing = true;
}

void CameraManager::PlayTrackForward(double dt)
{
  std::vector<CameraNode>& rNodes = m_tracks[m_selectedTrackIndex].nodes;

  double finalDt = dt * m_trackState.timeMultiplier;
  if (m_trackState.manualPlay)
  {
    InputManager* pInputManager = g_mainHandle->GetInputManager();
    finalDt *= pInputManager->GetActionState(InputManager::Action::Camera_Up) - pInputManager->GetActionState(InputManager::Action::Camera_Down);
  }
  m_trackState.time += finalDt;

  CameraNode* r2 = &rNodes[m_trackState.node];
  CameraNode* r3 = &rNodes[m_trackState.node + 1];

  if (m_trackState.time > r3->time)
  {
    if (m_trackState.node + 1 < rNodes.size() - 1)
    {
      m_trackState.node += 1;
      r2 = &rNodes[m_trackState.node];
      r3 = &rNodes[m_trackState.node + 1];
    }
    else
    {
      m_trackState.time = r3->time;
    }
  }
  else if (m_trackState.time < r2->time)
  {
    if (m_trackState.node > 0)
    {
      m_trackState.node -= 1;
      r2 = &rNodes[m_trackState.node];
      r3 = &rNodes[m_trackState.node + 1];
    }
    else
    {
      m_trackState.time = 0;
    }
  }

  CameraNode* r1 = (m_trackState.node > 0)                     ? &rNodes[m_trackState.node - 1] : r2;
  CameraNode* r4 = (m_trackState.node + 1 < rNodes.size() - 1) ? &rNodes[m_trackState.node + 2] : r3;

  double mu = (m_trackState.time - r2->time) / (r3->time - r2->time);
  XMVECTOR catmullPos = XMVectorCatmullRom(r1->vPos, r2->vPos, r3->vPos, r4->vPos, mu);
  XMVECTOR catmullRot = XMVectorCatmullRom(r1->qRot, r2->qRot, r3->qRot, r4->qRot, mu);

  XMMATRIX catmullMatrix = XMMatrixRotationQuaternion(XMQuaternionNormalize(catmullRot));
  catmullMatrix.r[3] = catmullPos;
  catmullMatrix.r[3].m128_f32[3] = 1.0f;

  m_trackState.fov = util::math::CatmullRomInterpolate(r1->fov, r2->fov, r3->fov, r4->fov, mu);
  m_trackState.transform = catmullMatrix;
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
  if (ImGui::ToggleButton(m_cameraEnabled ? "Disable" : "Enable", ImVec2(100, 25), m_cameraEnabled, true))
    ToggleCamera();
  ImGui::SameLine();
  if (ImGui::Button("Reset", ImVec2(100, 25)))
    ResetCamera();

  ImGui::PopFont();
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();
  ImGui::PushFont(io.Fonts->Fonts[4]);

  ImGui::Dummy(ImVec2(0, 0));
  ImGui::Columns(4, 0, false);
  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 12);

  ImGui::PushItemWidth(200);

  ImGui::Text("Movement speed");
  ImGui::InputFloat("##CameraMovementSpeed", &m_settings.movementSpeed, 0.1f, 1.0f, 2);
  ImGui::Text("Rotation speed");
  ImGui::InputFloat("##CameraRotationSpeed", &m_settings.rotationSpeed, 0.1f, 1.0f, 2);
  ImGui::Text("Roll speed");
  ImGui::InputFloat("##CameraRollSpeed", &m_settings.rollSpeed, 0.1f, 1.0f, 2);
  ImGui::Text("FoV speed");
  ImGui::InputFloat("##CameraFoVSpeed", &m_settings.zoomSpeed, 0.1f, 1.0f, 2);

  ImGui::Text("Game timescale");
  ImGui::SliderFloat("##TimeScale", &TD::TimeModule::Singleton()->m_TimeScale, 0, 1);

  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 290);
  ImGui::PushItemWidth(200);

  ImGui::Text("Agents");
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 10));
  ImGui::Combo("##PlayerList", &m_selectedPlayerIndex, m_playerList, m_playerCount);
  if (ImGui::Checkbox("Lock to agent", &m_lockToPlayer))
    ChangeTargetRelativity();
  ImGui::Checkbox("Shake camera", &m_shakeInfo.shakeEnabled);
  ImGui::PopStyleVar();

  ImGui::Text("Sway duration (s)");
  ImGui::InputFloat("##ShakeSpeed", &m_shakeInfo.swayDuration);
  ImGui::Text("Max angle");
  ImGui::InputFloat("##MaxRotation", &m_shakeInfo.maxAngle);

  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 552);
  ImGui::PushItemWidth(200);

  ImGui::Text("Camera tracks");
  if (ImGui::Combo("##CameraTrackList", &m_selectedTrackIndex, m_trackNameList, m_tracks.size()))
  {
    //GenerateDisplayNodes();
  }
  if (ImGui::Button("Create", ImVec2(95, 25)))
  {
    CreateTrack();
  }
  ImGui::SameLine(0, 10);
  if (ImGui::Button("Delete", ImVec2(95, 25)))
  {
    DeleteTrack();
  }
  ImGui::Dummy(ImVec2(0, 5));
  ImGui::Dummy(ImVec2(10, 0)); ImGui::SameLine(0, 0);
  ImGui::Separator(ImVec2(180, 1));
  ImGui::Dummy(ImVec2(0, 5));
  ImGui::Text("Time multiplier");

  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 10));
  ImGui::InputFloat("##CameraTrackSpeedMultiplier", &m_trackState.timeMultiplier, 0.1, 0, 2);

  ImGui::Dummy(ImVec2(0, 5));

  ImGui::Checkbox("Lock rotation", &m_trackState.rotationLocked);
  ImGui::Checkbox("Lock field of view", &m_trackState.fovLocked);
  ImGui::Checkbox("Play manually", &m_trackState.manualPlay);

  ImGui::PopStyleVar();
  ImGui::PopFont();
}

void CameraManager::UpdatePlayerList()
{
  m_pAgents.clear();

  TD::World* pWorld = TD::RogueClient::Singleton()->m_pClient->m_pWorld;
  if (pWorld)
  {
    if (pWorld->m_AgentArray && pWorld->m_AgentCount > 0)
    {
      for (int i = 0; i < pWorld->m_AgentCount; ++i)
      {
        TD::Agent* pAgent = pWorld->m_AgentArray[i];
        m_pAgents.push_back(pAgent);
      }
    }
  }

  const char** newAgentNames = new const char*[m_pAgents.size()];
  for (int i = 0; i < m_pAgents.size(); ++i)
    newAgentNames[i] = (const char*)(&m_pAgents[i]->m_Info->m_Name);

  const char** oldNames = m_playerList;
  m_playerList = newAgentNames;
  m_playerCount = m_pAgents.size();

  delete[] oldNames;
}

void CameraManager::ChangeTargetRelativity()
{
  if (m_lockToPlayer)
  {
    m_camera.position = XMVectorSet(0, 1.7f, -2.f, 1);
    m_camera.pitch = m_camera.yaw = m_camera.roll = 0;
    return;
  }

  XMMATRIX targetMatrix = m_pAgents[m_selectedPlayerIndex]->m_Transform;
  XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_camera.pitch), XMConvertToRadians(m_camera.yaw), 0);
  XMMATRIX rollMatrix = XMMatrixRotationRollPitchYaw(0, 0, XMConvertToRadians(m_camera.roll));
  rotationMatrix = XMMatrixMultiply(rollMatrix, rotationMatrix);

  XMVECTOR cameraPos = targetMatrix.r[3];
  cameraPos += m_camera.position.m128_f32[0] * targetMatrix.r[0];
  cameraPos += m_camera.position.m128_f32[1] * targetMatrix.r[1];
  cameraPos += m_camera.position.m128_f32[2] * targetMatrix.r[2];

  m_camera.position = cameraPos;
}

void CameraManager::GenerateShake(double dt)
{
  m_shakeInfo.dtShake += dt;
  if (m_shakeInfo.dtShake >= m_shakeInfo.swayDuration)
  {
    m_shakeInfo.dtShake = 0;
    float randPitch = fmod(rand(), m_shakeInfo.maxAngle * 2) - m_shakeInfo.maxAngle;
    float randYaw = fmod(rand(), m_shakeInfo.maxAngle * 2) - m_shakeInfo.maxAngle;

    m_shakeInfo.qRotations[0] = m_shakeInfo.qRotations[1];
    m_shakeInfo.qRotations[1] = m_shakeInfo.qRotations[2];
    m_shakeInfo.qRotations[2] = m_shakeInfo.qRotations[3];
    m_shakeInfo.qRotations[3] = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(randPitch), XMConvertToRadians(randYaw), 0);
  }

  double mu = m_shakeInfo.dtShake / m_shakeInfo.swayDuration;
  XMVECTOR qResult = XMVectorCatmullRom(m_shakeInfo.qRotations[0], m_shakeInfo.qRotations[1], m_shakeInfo.qRotations[2], m_shakeInfo.qRotations[3], mu);
  XMVECTOR vResult = XMVectorCatmullRom(m_shakeInfo.vSwayPositions[0], m_shakeInfo.vSwayPositions[1], m_shakeInfo.vSwayPositions[2], m_shakeInfo.vSwayPositions[3], mu);
  qResult = XMQuaternionNormalize(qResult);

  m_shakeInfo.shakeMatrix = XMMatrixRotationQuaternion(qResult);
}
