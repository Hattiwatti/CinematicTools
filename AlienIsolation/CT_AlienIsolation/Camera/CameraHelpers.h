#pragma once
#include <Windows.h>
#include <DirectXMath.h>

struct GamepadState
{
  DirectX::XMFLOAT2 leftStick;
  DirectX::XMFLOAT2 rightStick;
  double trigger;
  BYTE leftShoulder;
  BYTE rightShoulder;
  BYTE leftThumbButton;
  BYTE rightThumbButton;
  BYTE dpad_Left;
  BYTE dpad_Right;
  BYTE dpad_Up;
  BYTE dpad_Down;
  BYTE Buttons[4];
};

struct ControlState
{
  float dX, dY, dZ;
  float dPitch, dYaw, dRoll;
};

struct Camera
{
  float x, y, z;
  float pitch, yaw, roll;
  float fov;

  float movementSpeed{ 1.0f };
  float rotationSpeed{ DirectX::XM_PI/4 };
  float rollSpeed{ DirectX::XM_PI / 8 };

  DirectX::XMFLOAT4 qRotation;
};

struct RotationBuffer
{
  double pitch[100];
  double yaw[100];
  double roll[100];
};