#pragma once
#include "TrackPlayer.h"
#include "../inih/cpp/INIReader.h"
#include "../Foundation.h"

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

  void OnCameraUpdate(Foundation::GameRender*);

  void HotkeyUpdate();
  void Update(double dt);
  void DrawUI();
  void DrawTrack() { m_TrackPlayer.DrawNodes(); }

  bool IsCameraEnabled() const { return m_CameraEnabled; }
  bool IsGamepadDisabled() const { return m_CameraEnabled && m_GamepadDisabled; };
  bool IsKbmDisabled() const { return m_CameraEnabled && m_KbmDisabled; };

  Camera& GetCamera() { return m_Camera; }

  void ReadConfig(INIReader* pReader);
  const std::string GetConfig() const;

private:
  void UpdateCamera(double dt);
  void UpdateInput(double dt);

  void ToggleCamera();
  void ResetCamera();

private:
  bool m_CameraEnabled;
  bool m_FirstEnable;
  bool m_AutoReset;
  bool m_UIRequestReset;

  CameraInput m_CameraInput;
  bool m_GamepadDisabled;
  bool m_KbmDisabled;

  Camera m_Camera;
  TrackPlayer m_TrackPlayer;

  boost::chrono::high_resolution_clock::time_point m_dtCameraUpdate;
  MouseBuffer m_MouseBuffer;
  bool m_SmoothMouse;

public:
  CameraManager(CameraManager const&) = delete;
  void operator=(CameraManager const&) = delete;
};