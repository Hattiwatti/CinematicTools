#include "CameraHelpers.h"

class CameraManager
{
public:
  CameraManager();
  ~CameraManager();

  void Update(double dt);
  bool CameraHook(int pCamera);

  bool IsCameraEnabled() { return m_cameraEnabled; }

private:
  bool m_cameraEnabled;

  Camera m_camera;

public:
  CameraManager(CameraManager const&) = delete;
  void operator=(CameraManager const&) = delete;
};