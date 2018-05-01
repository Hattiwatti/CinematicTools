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
  bool IsTimeFrozen() { return m_TimeFreezeEnabled; }
  
  bool OverrideRatio() { return m_OverrideAspectRatio; }
  float GetAspectRatio() { return m_AspectRatio; }

  DirectX::XMFLOAT4X4& GetCameraTrans() { return m_Camera.Transform; }
  DirectX::XMMATRIX& GetMatrix() { return m_Camera.Matrix; }

  float GetCameraFov() { return DirectX::XMConvertToRadians(m_Camera.FieldOfView); }
  void SetResetMatrix(DirectX::XMFLOAT4X4 const& m) { m_ResetMatrix = m; }

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
  bool m_AutoReset;

  bool m_GamepadDisabled;
  bool m_KbmDisabled;

  bool m_TimeFreezeEnabled;
  bool m_OverrideAspectRatio;
  float m_AspectRatio;

  bool m_UiRequestReset;
  DirectX::XMFLOAT4X4 m_ResetMatrix;

  Camera m_Camera;
  DepthOfField m_DepthOfField;
  TrackPlayer m_TrackPlayer;

public:
  CameraManager(CameraManager const&) = delete;
  void operator=(CameraManager const&) = delete;
};