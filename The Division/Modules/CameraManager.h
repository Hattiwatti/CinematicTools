#pragma once
#include <DirectXMath.h>
#include <string>
#include <vector>

#include "Snowdrop.h"

using namespace DirectX;

struct CameraNode
{
  XMVECTOR vPos;
  XMVECTOR qRot;
  float fov;
  float time;
};

struct Camera
{
  XMVECTOR position{ XMVectorZero() };
  float pitch{ 0 };
  float yaw{ 0 };
  float roll{ 0 };
  float fov{ 60.f };
};

struct CameraTrack
{
  std::string name;
  std::vector<CameraNode> nodes;
};

struct TrackState
{
  XMMATRIX transform{ XMMatrixIdentity() };
  float fov{ 30.f };
  double time{ 0 };
  float timeMultiplier{ 1.0f };
  int node{ 0 };
  bool playing{ false };
  bool rotationLocked{ true };
  bool fovLocked{ true };
  bool manualPlay{ false };
};

struct CameraSettings
{
  float movementSpeed{ 5.0f };
  float rotationSpeed{ 45.f };
  float rollSpeed{ 10.f };
  float zoomSpeed{ 5.f };
};

struct CameraShake
{
  bool shakeEnabled{ false };
  float maxAngle{ 5.0f };
  float maxDistance{ 0.5f };
  float swayDuration{ 2.0f };

  XMVECTOR qRotations[4];
  XMVECTOR vSwayPositions[4];

  XMMATRIX shakeMatrix{ XMMatrixIdentity() };
  float dtShake{ 0.f };
};

class CameraManager
{
public:
  CameraManager();
  ~CameraManager();

  void CameraHook(__int64 pCamera);
  void Update(double);

  void ToggleCamera();
  void DrawUI();

private:
  void ResetCamera();
  void ChangeTargetRelativity();
  void GenerateShake(double);

  void CreateTrack();
  void DeleteTrack();
  void CreateNode();
  void DeleteNode();

  void PlayTrackForward(double);
  void ToggleTrackPlay();

  void UpdatePlayerList();

private:
  bool m_cameraEnabled;
  bool m_firstEnable;
  bool m_lockToPlayer;

  Camera m_camera;
  CameraSettings m_settings;
  CameraShake m_shakeInfo;

  std::vector<CameraTrack> m_tracks;
  const char** m_trackNameList;
  int m_selectedTrackIndex;

  TrackState m_trackState;
  int m_runningId;

  const char** m_playerList;
  std::vector<TD::Agent*> m_pAgents;
  int m_playerCount;
  int m_selectedPlayerIndex;

public:
  CameraManager(CameraManager const&) = delete;
  void operator=(CameraManager const&) = delete;

};