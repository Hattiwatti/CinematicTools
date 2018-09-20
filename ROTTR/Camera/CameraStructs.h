#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <VertexTypes.h>
#include <wrl.h>

struct CatmullRomDof
{
  float Strength;
  float FocusDistance;
  float NearStart;
  float NearEnd;
  float FarStart;
  float FarEnd;
  bool Enabled;
};

struct CatmullRomNode
{
  DirectX::XMFLOAT3 Position;
  DirectX::XMFLOAT4 Rotation;
  DirectX::XMMATRIX Transform;
  float FocalLength;
  float Aperture;
  float FocusDistance;
  float TimeStamp;
};

struct Camera
{
  DirectX::XMFLOAT3 Position{ 0,0,0 };
  DirectX::XMFLOAT4 Rotation{ 0,0,0,1 };
  float FocalLength{ 30.f };
  float Aperture{ 4.f };
  float FocusDistance{ 10.f };

  float MovementSpeed{ 1.0f };
  float RotationSpeed{ DirectX::XM_PI/4 };
  float RollSpeed{ DirectX::XM_PI / 8 };
  float FocalSpeed{ 5.0f };
  float ApertureSpeed{ 1.0f };
  float FocusSpeed{ 1.0f };

  DirectX::XMFLOAT4X4 Transform{  1,0,0,0,
                                  0,1,0,0,
                                  0,0,1,0,
                                  0,0,0,1 };
};

struct CameraInput
{
  float dX{ 0 };
  float dY{ 0 };
  float dZ{ 0 };
  float dPitch{ 0 };
  float dYaw{ 0 };
  float dRoll{ 0 };
  float dFocalLength{ 0 };
  float dFocusDistance{ 0 };
  float dAperture{ 0 };

  void Clear()
  {
    dX = 0;
    dY = 0;
    dZ = 0;
    dPitch = 0;
    dYaw = 0;
    dRoll = 0;
    dFocalLength = 0;
    dFocusDistance = 0;
    dAperture = 0;
  }
};

struct CameraTrack
{
  std::string Name;
  std::vector<CatmullRomNode> Nodes;
  Microsoft::WRL::ComPtr<ID3D11Buffer> Vertices;
  Microsoft::WRL::ComPtr<ID3D11Buffer> Indices;
  unsigned int IndexCount{ 0 };

  CameraTrack(std::string const& name)
  {
    Name = name;
  }
};