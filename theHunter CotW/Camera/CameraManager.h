#pragma once
#include "TrackPlayer.h"
#include "../inih/cpp/INIReader.h"

class CameraManager
{
public:
  CameraManager();
  ~CameraManager();

  void HotkeyUpdate();
  void Update(double dt);
  void DrawUI();

  bool IsCameraEnabled() { return m_CameraEnabled; }
  bool IsGamepadDisabled() { return m_CameraEnabled && m_GamepadDisabled; };
  bool IsKbmDisabled() { return m_CameraEnabled && m_KbmDisabled; };

  void ReadConfig(INIReader* pReader);
  const std::string GetConfig();

private:
  void UpdateCamera(double dt);
  void UpdateInput(double dt);

  void ToggleCamera();
  void ResetCamera();

private:
  bool m_CameraEnabled;
  bool m_FirstEnable;

  bool m_GamepadDisabled;
  bool m_KbmDisabled;

  Camera m_Camera;
  TrackPlayer m_TrackPlayer;

public:
  CameraManager(CameraManager const&) = delete;
  void operator=(CameraManager const&) = delete;
};