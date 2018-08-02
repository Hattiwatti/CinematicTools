#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <VertexTypes.h>
#include <wrl.h>

struct CatmullRomNode
{
  DirectX::XMFLOAT3 Position;
  DirectX::XMFLOAT4 Rotation;
  DirectX::XMMATRIX Transform;
  float FieldOfView;
  float FocusDistance;
  float DofScale;
  float DofStrength;
  float TimeStamp;
};

struct CameraProfile
{
  std::string Name{ "Default" };
  float FieldOfView{ 50.f };
  float MovementSpeed{ 1.0f };
  float RotationSpeed{ DirectX::XM_PI / 4 };
  float RollSpeed{ DirectX::XM_PI / 8 };
  float FovSpeed{ 5.0f };
  float DofScale{ 1.0f };
  float DofStrength{ 0.04f };
  float FocusDistance{ 2.f };
};

struct Camera
{
  CameraProfile Profile;

  DirectX::XMFLOAT3 Position{ 0,0,0 };
  DirectX::XMFLOAT4 Rotation{ 0,0,0,1 };

  float dX{ 0 };
  float dY{ 0 };
  float dZ{ 0 };
  float dPitch{ 0 };
  float dYaw{ 0 };
  float dRoll{ 0 };
  float dFov{ 0 };
  float dFocus{ 0 };
  float dDofStrength{ 0 };
  float dDofScale{ 0 };

  DirectX::XMFLOAT3 AbsolutePosition{ 0,0,0 };
  DirectX::XMFLOAT4 AbsoluteRotation{ 0,0,0,1 };
  DirectX::XMFLOAT4X4 TargetMatrix{ 1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1 };
};

// Structure for interpolating irregularly timed nodes
struct SmoothNode
{
  float Time;
  float Value;
};

struct CameraTrack
{
  std::string Name;
  std::vector<CatmullRomNode> Nodes;
  std::vector<SmoothNode> SmoothNodes;
  Microsoft::WRL::ComPtr<ID3D11Buffer> Vertices;
  Microsoft::WRL::ComPtr<ID3D11Buffer> Indices;
  unsigned int IndexCount{ 0 };

  CameraTrack(std::string const& name)
  {
    Name = name;
  }
};
