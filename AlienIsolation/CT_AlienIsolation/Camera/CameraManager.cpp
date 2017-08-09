#include "CameraManager.h"
#include "../Util/Log.h"

using namespace DirectX;

CameraManager::CameraManager()
{
  m_camera.pitch = m_camera.yaw = m_camera.roll = 0;
  XMVECTOR rotation = DirectX::XMQuaternionRotationRollPitchYaw(0, 0, 0);
  XMStoreFloat4(&m_camera.qRotation, rotation);
  m_camera.x = 2.06f;
  m_camera.y = 14.85f;
  m_camera.z = 37.f;

  m_cameraEnabled = true;
  Log::Ok("Camera Manager initialized");
}

bool CameraManager::CameraHook(int pCamera)
{
  if (!m_cameraEnabled) return false;

  XMFLOAT4* pRotation = (XMFLOAT4*)(pCamera);
  XMFLOAT3* pTransform = (XMFLOAT3*)(pCamera + 0x10);

  *pRotation = m_camera.qRotation;
  pTransform->x = m_camera.x;
  pTransform->y = m_camera.y;
  pTransform->z = m_camera.z;

  return true;
}

CameraManager::~CameraManager()
{
  
}