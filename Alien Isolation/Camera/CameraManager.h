#pragma once
#include "TrackPlayer.h"
#include "../inih/cpp/INIReader.h"
#include "../AlienIsolation.h"

#include <array>
#include <boost/chrono/chrono.hpp>

struct MouseBuffer
{
  std::array<DirectX::XMFLOAT3, 25> Values{ DirectX::XMFLOAT3(0,0,0) };

  void AddValue(DirectX::XMFLOAT3 const& val)
  {
    for (int i = 24; i > 0; i -= 1)
      Values[i] = Values[i - 1];

    Values[0] = val;
  }

  DirectX::XMFLOAT3 CalcAverage()
  {
    DirectX::XMFLOAT3 avg{ 0,0,0 };
    for (int i = 0; i < 25; ++i)
    {
      avg.x += Values[i].x / 25;
      avg.y += Values[i].y / 25;
      avg.z += Values[i].z / 25;
    }

    return avg;
  }
};

class CameraManager
{
public:
  CameraManager();
  ~CameraManager();

  // We need 2 hooks, so our camera is only used to draw the world
  // Original camera state is saved on Begin() and then restored on End()
  // This fixes a bug where locking on Amanda would result to continuous
  // rotation because she would always face where the camera was pointing.
  void OnCameraUpdateBegin();
  void OnCameraUpdateEnd();

  void OnPostProcessUpdate(CATHODE::PostProcess* pPostProcess);
  void OnMapChange();

  void HotkeyUpdate();
  void Update(float dt);
  void DrawUI();
  void DrawTrack() { if(m_CameraEnabled) m_TrackPlayer.DrawNodes(); }

  bool IsCameraEnabled() { return m_CameraEnabled; }
  bool IsGamepadDisabled() { return m_CameraEnabled && m_GamepadDisabled; };
  bool IsKbmDisabled() { return m_CameraEnabled && m_KbmDisabled; };

  void ReadConfig(INIReader* pReader);
  const std::string GetConfig();

  Camera const& GetCamera() { return m_Camera; }

private:
  // Updates camera position and rotation
  void UpdateCamera(float dt);

  // Updates camera input states
  void UpdateInput(float dt);

  void ToggleCamera();
  void ResetCamera();

  // Changes camera coordinates between global and
  // target relative space
  void ChangeCamRelativity();

  // Gets target character transform
  XMMATRIX GetTargetMatrix();

  // Creates a new profile based on current camera settings
  void CreateProfile();

  // Loads camera profiles from files
  void LoadProfiles();
  // Saves camera profiles to files
  void SaveProfiles();

  void ToggleHUD();

private:
  bool m_CameraEnabled;
  bool m_FirstEnable;
  bool m_AutoReset;
  bool m_UIRequestReset;

  bool m_GamepadDisabled;
  bool m_KbmDisabled;

  bool m_LockToCharacter;
  unsigned int m_CharacterIndex;
  CATHODE::Character* m_pCharacter;

  bool m_HideUI;

  Camera m_Camera;
  TrackPlayer m_TrackPlayer;

  boost::chrono::high_resolution_clock::time_point m_dtCameraUpdate;
  MouseBuffer m_MouseBuffer;
  bool m_SmoothMouse;

  bool m_ShowProfileModal;
  char m_ModalProfileName[50];
  std::vector<CameraProfile> m_Profiles;
  int m_SelectedProfile;

  float m_TimeScale;

public:
  CameraManager(CameraManager const&) = delete;
  void operator=(CameraManager const&) = delete;
};