#pragma once
#include <DirectXMath.h>
#include <vector>

struct Camera
{
  DirectX::XMFLOAT3 position{ 0,0,0 };
  DirectX::XMFLOAT4 rotation{ 0,0,0,1 };
  float roll{ 0.f };
  float fov{ 60.f };

  float nearPlane{ 0.25f };
  float farPlane{ 10000.f };

  float dX{ 0 }, dY{ 0 }, dZ{ 0 };
  float dPitch{ 0 }, dYaw{ 0 }, dRoll{ 0 };
  float dFov{ 0 };

  float movementSpeed{ 1.f };
  float rotationSpeed{ DirectX::XMConvertToRadians(45.f) };
  float rollSpeed{ DirectX::XMConvertToRadians(10.f) };
  float fovSpeed{ 2.5f };

  DirectX::XMFLOAT4X4 rotMatrix{ 1,0,0,0,
                                 0,1,0,0,
                                 0,0,1,0,
                                 0,0,0,1 };
};

struct DepthOfField
{
  float focusDistance{ 10.f };
  float farDistance{ 5.f };
  float nearDistance{ -5.f };
  float cocSize{ 0.3f };
  bool enabled{ false };
};

struct SmoothNode
{
  double time;
  double value;
};

struct CatmullRomDof
{
  bool enabled;
  float strength;
  float focusDistance;
  float nearStart;
  float nearEnd;
  float farStart;
  float farEnd;
};

struct CatmullRomNode
{
  DirectX::XMFLOAT3 vPosition;
  DirectX::XMFLOAT4 qRotation;
  float roll;
  float time;
  float fov;
  CatmullRomDof dof;
  DirectX::XMFLOAT4X4 rotMatrix;
};

struct DisplayNode
{
  DirectX::XMFLOAT3 pos;
  DirectX::XMFLOAT3 forward;
};

struct CameraTrack
{
  std::string name;
  std::vector<CatmullRomNode> nodes;
  //std::vector<SmoothNode> smoothNodes;
};