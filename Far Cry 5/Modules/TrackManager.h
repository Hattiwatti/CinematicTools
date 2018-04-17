#pragma once
#include "CameraStructs.h"

#include <boost/thread.hpp>
#include <memory>
#include <mutex>
#include <VertexTypes.h>

class TrackManager
{
  struct PlayState
  {
    unsigned int node;
    //int smoothNode;
    double time;
  };

public:
  TrackManager();
  ~TrackManager();

  void Update(double dt);

  void HotkeyUpdate();
  void DrawUI();
  void DrawNodes();

  bool IsRotationLocked() { return m_lockRotation; }
  bool IsFovLocked() { return m_lockFov; }
  bool IsPlaying() { return m_playing; }
  void Play();

  void CreateNode(const Camera& camera);
  void DeleteNode();
  CatmullRomNode PlayForward(double dt, bool ignoreManual = false);

private:
  bool m_playing;
  bool m_lockRotation; // Use rotation from track
  bool m_lockFov;
  bool m_manualPlay; // Let player move through track with gamepad triggers

  float m_speedMultiplier;

  std::vector<CameraTrack> m_tracks;
  unsigned int m_selectedTrack;
  unsigned int m_selectedNode;

  PlayState m_state;

  //std::unique_ptr<DirectX::Model> m_cameraModel;
  //CMOWrapper* m_pCameraModel;
  std::vector<DisplayNode> m_displayNodes;

  DirectX::VertexPositionColor* m_pDisplayVertices;
  uint16_t* m_pDisplayIndices;
  size_t m_vertexCount;
  size_t m_indexCount;

  //void SmoothTrack();
  void GenerateDisplayNodes();
  boost::mutex m_nodeMutex; // So we don't try to draw a node that's being deleted for example.
  boost::mutex m_displayMutex; // Also don't draw while generating display nodes

  const char** m_trackList;
  int m_runningId; // Number that's used in the name of the new track ("Track #(runningId++)")

  void CreateTrack();
  void DeleteTrack();

  // If there are no nodes, provide a dummy node for UI
  CatmullRomNode m_dummyNode;

public:
  TrackManager(TrackManager const&) = delete;
  void operator=(TrackManager const&) = delete;
};