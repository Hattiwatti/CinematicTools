#pragma once
#include "../Dunya.h"
#include "TrackManager.h"

class CameraManager
{
public:
  CameraManager();
  ~CameraManager();

  void CameraHook(FC::CMarketingCamera* pGameCamera);
  void AngleHook(__int64 pMatrix);
  void __fastcall ComponentHook(FC::ComponentCollection<__int64>* pCollection);

  void HotkeyUpdate();
  void Update(double dt);
  void DrawUI();

  bool IsCameraEnabled() { return m_CameraEnabled; }
  bool IsGamepadDisabled() { return m_GamepadDisabled; }
  bool IsGameUIDisabled() { return m_GameUIDisabled; }

  DepthOfField& GetDoF() { return m_Dof; }
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

  bool m_GameUIDisabled;

  Camera m_Camera;
  DepthOfField m_Dof;
  FC::CMarketingCameraActivator* m_pActivator;
  FC::CMarketingCamera* m_pGameCamera;

  bool m_uiRequestToggle;
  bool m_uiRequestReset;

  TrackManager m_TrackManager;
  float m_mousePitchBuffer[50];
  float m_mouseYawBuffer[50];
  float m_dtBufferUpdate;

public:
  CameraManager(CameraManager const&) = delete;
  void operator=(CameraManager const&) = delete;
};