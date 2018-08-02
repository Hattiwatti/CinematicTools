#pragma once
#include "CameraStructs.h"
#include <Model.h>
#include <memory>
#include <vector>

class TrackPlayer
{
public:
  TrackPlayer();
  ~TrackPlayer();

  void CreateNode(Camera const& camera);
  void DeleteNode();

  void Toggle();
  CatmullRomNode PlayForward(float dt, bool ignoreManual = false);
  CatmullRomNode PlayForwardSmooth(float dt, bool ignoreManual = false);

  void DrawUI();
  void DrawNodes();

  bool IsPlaying() { return m_IsPlaying; }
  bool IsRotationLocked() { return m_LockRotation; }
  bool IsFovLocked() { return m_LockFieldOfView; }
  bool IsDofLocked() { return m_LockDepthOfField; }

private:
  void CreateTrack();
  void DeleteTrack();

  void UpdateNodeBuffers();
  void UpdateNameList();

  void SmoothTrack();

private:
  bool m_IsPlaying;

  bool m_LockDepthOfField;
  bool m_LockRotation;
  bool m_LockFieldOfView;
  bool m_ManualPlay;
  float m_NodeTimeSpan;

  unsigned int m_CurrentNode;
  unsigned int m_CurrentSmoothNode;
  float m_CurrentTime;

  std::vector<CameraTrack> m_Tracks;
  unsigned int m_SelectedTrack;

  std::vector<const char*> m_TrackNames;
  int m_RunningId;

  std::unique_ptr<DirectX::Model> m_pCameraModel;

public:
  TrackPlayer(TrackPlayer const&) = delete;
  void operator=(TrackPlayer const&) = delete;

};