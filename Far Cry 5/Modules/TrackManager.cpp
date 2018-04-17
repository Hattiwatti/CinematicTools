#include "TrackManager.h"
#include "../../Main.h"
#include "../../resource.h"
#include "../../Util/Util.h"
#include "../../Util/ImGuiHelpers.h"

using namespace DirectX;

TrackManager::TrackManager()
{
  m_tracks.clear();
  m_selectedNode = 0;
  m_selectedTrack = 0;

  m_state.node = 0;
  m_state.time = 0;
  m_speedMultiplier = 1.0f;

  m_playing = false;
  m_lockRotation = true;
  m_manualPlay = false;

  CameraTrack defaultTrack;
  defaultTrack.name = "Track #1";
  defaultTrack.nodes.clear();
  m_runningId = 2;

  m_tracks.push_back(defaultTrack);
  m_trackList = new const char*[1];
  m_trackList[0] = (const char*)m_tracks[0].name.c_str();

  m_dummyNode.time = 0;

  void* pModelData;
  DWORD szData;

  //if (util::GetResource(IDR_CAMERACMO, pModelData, szData))
  //  m_pCameraModel = g_mainHandle->GetRenderer()->CreateModel(pModelData, szData);
  //else
  //  util::log::Warning("Could not get camera.cmo resource");

  m_pDisplayVertices = nullptr;
  m_pDisplayIndices = nullptr;
  m_vertexCount = 0;
  m_indexCount = 0;
}

void TrackManager::HotkeyUpdate()
{

}

CatmullRomNode TrackManager::PlayForward(double dt, bool ignoreManual /*= false*/)
{
  std::vector<CatmullRomNode>& nodes = m_tracks[m_selectedTrack].nodes;

  CatmullRomNode resultNode;
  if (!m_manualPlay || ignoreManual)
    m_state.time += dt * m_speedMultiplier;
  else
  {
    float timeMultiplier = g_mainHandle->GetInputManager()->GetActionState(Action::Camera_Up);
    timeMultiplier -= g_mainHandle->GetInputManager()->GetActionState(Action::Camera_Down);
    m_state.time += dt * timeMultiplier * m_speedMultiplier;
  }

  std::vector<CatmullRomNode>::iterator n0, n1, n2, n3;

  bool goForward = false; // Time is increasing, should we move on to the next nodes?
  bool goBackward = false; // Time is decreasing, should we move on to the previous nodes?

  while ((goForward = m_state.time >= nodes[m_state.node + 1].time) ||
    (goBackward = m_state.time < nodes[m_state.node].time))
  {
    if (goForward)
    {
      if (m_state.node + 1 < nodes.size() - 1)
        m_state.node++;
      else
      {
        resultNode.qRotation = nodes[m_state.node + 1].qRotation;
        resultNode.vPosition = nodes[m_state.node + 1].vPosition;
        resultNode.fov = nodes[m_state.node + 1].fov;
        return resultNode;
      }
    }
    else if (goBackward)
    {
      if (m_state.node - 1 >= 0)
      {
        m_state.node -= 1;
      }
      else
      {
        m_state.time = 0;
        resultNode.qRotation = nodes[0].qRotation;
        resultNode.vPosition = nodes[0].vPosition;
        resultNode.fov = nodes[0].fov;
        return resultNode;
      }
    }
  }

  n1 = nodes.begin() + m_state.node;
  n2 = n1 + 1;
  n0 = m_state.node > 0 ? n1 - 1 : n1;
  n3 = m_state.node + 1 < nodes.size() - 1 ? n2 + 1 : n2;

  double mu = (m_state.time - n1->time) / (n2->time - n1->time);

  XMVECTOR qRot0 = XMLoadFloat4(&n0->qRotation);
  XMVECTOR qRot1 = XMLoadFloat4(&n1->qRotation);
  XMVECTOR qRot2 = XMLoadFloat4(&n2->qRotation);
  XMVECTOR qRot3 = XMLoadFloat4(&n3->qRotation);

  XMVECTOR vPos0 = XMLoadFloat3(&n0->vPosition);
  XMVECTOR vPos1 = XMLoadFloat3(&n1->vPosition);
  XMVECTOR vPos2 = XMLoadFloat3(&n2->vPosition);
  XMVECTOR vPos3 = XMLoadFloat3(&n3->vPosition);
  resultNode.fov = util::math::CatmullRomInterpolate(n0->fov, n1->fov, n2->fov, n3->fov, mu);

  XMVECTOR resultRot = XMVectorCatmullRom(qRot0, qRot1, qRot2, qRot3, mu);
  XMVECTOR resultPos = XMVectorCatmullRom(vPos0, vPos1, vPos2, vPos3, mu);

  XMStoreFloat4(&resultNode.qRotation, XMQuaternionNormalize(resultRot));
  XMStoreFloat3(&resultNode.vPosition, resultPos);
  return resultNode;
}

void TrackManager::Play()
{
  if (m_tracks[m_selectedTrack].nodes.size() < 2)
  {
    util::log::Warning("Can't play a camera track with less than 2 nodes");
    return;
  }

  m_playing = !m_playing;
  if (!m_playing) return;

  m_state.node = 0;
  //m_state.smoothNode = 0;
  m_state.time = 0;
}

void TrackManager::DrawNodes()
{
  if (m_playing) return;

  if (m_nodeMutex.try_lock())
  {
    std::vector<CatmullRomNode>& nodes = m_tracks[m_selectedTrack].nodes;
    for (auto& itr : nodes)
    {
      //g_mainHandle->GetRenderer()->DrawCMOModel(m_pCameraModel, XMLoadFloat4x4(&itr.rotMatrix));
    }
    m_nodeMutex.unlock();
  }

  if (m_indexCount > 0 && m_vertexCount > 0)
  {
    if (m_displayMutex.try_lock())
    {
      //g_mainHandle->GetRenderer()->DrawBuffer(m_pDisplayIndices, m_indexCount, m_pDisplayVertices, m_vertexCount);
      m_displayMutex.unlock();
    }
  }
}

void TrackManager::CreateNode(const Camera& camera)
{
  if (m_playing) return;

  m_nodeMutex.lock();

  CatmullRomNode newNode;
  newNode.fov = camera.fov;

  XMVECTOR qRot = XMLoadFloat4(&camera.rotation);
  newNode.vPosition = camera.position;
  XMStoreFloat4(&newNode.qRotation, XMQuaternionNormalize(qRot));
  XMMATRIX rotMatrix = XMMatrixRotationQuaternion(qRot);
  rotMatrix.r[3] = XMLoadFloat3(&camera.position);
  rotMatrix.r[3].m128_f32[3] = 1.0f;
  XMStoreFloat4x4(&newNode.rotMatrix, rotMatrix);
  newNode.time = 0;

  int nodes = m_tracks[m_selectedTrack].nodes.size();
  if (nodes > 0)
    newNode.time = m_tracks[m_selectedTrack].nodes[nodes - 1].time + 3.0f;

  m_tracks[m_selectedTrack].nodes.push_back(newNode);
  m_selectedNode = m_tracks[m_selectedTrack].nodes.size() - 1;

  m_nodeMutex.unlock();
  //GenerateDisplayNodes();

  util::log::Write("Node created, total nodes: %d", m_tracks[m_selectedTrack].nodes.size());
}

void TrackManager::DeleteNode()
{
  if (m_playing) return;

  std::vector<CatmullRomNode>& nodes = m_tracks[m_selectedTrack].nodes;
  if (nodes.size() == 0) return;

  if (m_selectedNode > 0)
    m_selectedNode -= 1;

  m_nodeMutex.lock();
  nodes.erase(nodes.begin() + (nodes.size() - 1));
  m_nodeMutex.unlock();

  util::log::Write("Deleted node, remaining nodes %d", nodes.size());
  //GenerateDisplayNodes();
}

void TrackManager::DrawUI()
{
  ImGui::Text("Camera tracks");
  if (ImGui::Combo("##CameraTrackList", (int*)&m_selectedTrack, m_trackList, m_tracks.size()))
    GenerateDisplayNodes();
  if (ImGui::Button("Create", ImVec2(95, 25)))
    CreateTrack();
  ImGui::SameLine(0, 10);
  if (ImGui::Button("Delete", ImVec2(95, 25)))
    DeleteTrack();
  ImGui::Dummy(ImVec2(0, 5));
  ImGui::Dummy(ImVec2(10, 0)); ImGui::SameLine(0, 0);
  ImGui::Separator(ImVec2(180, 1));
  ImGui::Dummy(ImVec2(0, 5));
  ImGui::Text("Time multiplier");
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 10));
  ImGui::InputFloat("##CameraTrackSpeedMultiplier", &m_speedMultiplier, 0.1, 0, 2);

  ImGui::Dummy(ImVec2(0, 5));

  ImGui::Checkbox("Lock rotation", &m_lockRotation);
  ImGui::Checkbox("Lock field of view", &m_lockFov);
  ImGui::Checkbox("Play manually", &m_manualPlay);
  ImGui::PopStyleVar();
}

void TrackManager::CreateTrack()
{
  if (m_playing) return;

  CameraTrack newTrack;
  newTrack.name = "Track #" + std::to_string(m_runningId++);
  newTrack.nodes.clear();

  m_tracks.push_back(newTrack);

  const char** oldTrackList = m_trackList;
  const char** newTrackList = new const char*[m_tracks.size()];

  for (unsigned int i = 0; i<m_tracks.size(); ++i)
    newTrackList[i] = m_tracks[i].name.c_str();

  m_trackList = newTrackList;
  delete[] oldTrackList;

  m_selectedTrack = m_tracks.size() - 1;
  //GenerateDisplayNodes();
}

void TrackManager::DeleteTrack()
{
  if (m_playing || m_tracks.size() <= 1) return;

  m_nodeMutex.lock();
  m_tracks[m_selectedTrack].nodes.clear();
  m_tracks.erase(m_tracks.begin() + m_selectedTrack);

  if (m_selectedTrack >= m_tracks.size())
    m_selectedTrack -= 1;

  m_nodeMutex.unlock();

  //delete[] m_trackList;
  const char** oldTrackList = m_trackList;
  const char** newTrackList = new const char*[m_tracks.size()];

  for (unsigned int i = 0; i<m_tracks.size(); ++i)
    newTrackList[i] = m_tracks[i].name.c_str();

  m_trackList = newTrackList;
  delete[] oldTrackList;

  //GenerateDisplayNodes();
}

TrackManager::~TrackManager()
{

}

void TrackManager::GenerateDisplayNodes()
{
  const std::vector<CatmullRomNode>& nodes = m_tracks[m_selectedTrack].nodes;

  m_displayMutex.lock();
  m_displayNodes.clear();

  if (m_pDisplayVertices)
  {
    delete[] m_pDisplayVertices;
    m_pDisplayVertices = nullptr;
    m_vertexCount = 0;
  }

  if (m_pDisplayIndices)
  {
    delete[] m_pDisplayIndices;
    m_pDisplayIndices = nullptr;
    m_indexCount = 0;
  }

  if (nodes.size() < 2)
  {
    m_displayMutex.unlock();
    return;
  }

  std::vector<VertexPositionColor> verticesVector;
  std::vector<unsigned int> indicesVector;

  VertexPositionColor v1, v2;
  v1.position = nodes[0].vPosition;
  v1.color = v2.color = XMFLOAT4(1, 0, 0, 1);

  XMVECTOR forward = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), XMLoadFloat4(&nodes[0].qRotation));
  XMStoreFloat3(&v2.position, XMLoadFloat3(&nodes[0].vPosition) - forward);

  verticesVector.push_back(v1);
  verticesVector.push_back(v2);
  indicesVector.push_back(0);
  indicesVector.push_back(1);

  m_state.time = 0;
  m_state.node = 0;
  int lastNodeVertexIndex = 0;
  int stepCount = 0;

  while (m_state.time < nodes[nodes.size() - 1].time)
  {
    stepCount++;

    VertexPositionColor nodeVertex, forwardVertex;
    nodeVertex.color = forwardVertex.color = XMFLOAT4(1, 0, 0, 1);

    CatmullRomNode node = PlayForward(0.1f, true);
    nodeVertex.position = node.vPosition;

    verticesVector.push_back(nodeVertex);
    indicesVector.push_back(lastNodeVertexIndex); // Previous node vertex
    indicesVector.push_back(verticesVector.size() - 1); // Current node vertex

    lastNodeVertexIndex = verticesVector.size() - 1;

    // Create forward line to indicate 0.5 second of track time
    if (stepCount % 5 == 0)
    {

      XMVECTOR forward = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), XMLoadFloat4(&node.qRotation));
      XMStoreFloat3(&forwardVertex.position, XMLoadFloat3(&node.vPosition) - 0.5*forward);

      verticesVector.push_back(forwardVertex);
      indicesVector.push_back(verticesVector.size() - 2);
      indicesVector.push_back(verticesVector.size() - 1);
    }
  }

  m_pDisplayVertices = new VertexPositionColor[verticesVector.size()];
  m_pDisplayIndices = new uint16_t[indicesVector.size()];

  for (unsigned int i = 0; i < verticesVector.size(); ++i)
    m_pDisplayVertices[i] = verticesVector[i];
  for (unsigned int i = 0; i < indicesVector.size(); ++i)
    m_pDisplayIndices[i] = indicesVector[i];

  m_vertexCount = verticesVector.size();
  m_indexCount = indicesVector.size();

  m_state.time = 0;
  m_state.node = 0;
  m_displayMutex.unlock();
}
