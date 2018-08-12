#pragma once
#include <Windows.h>
#include <DirectXMath.h>
#include <Xinput.h>

using namespace DirectX;

struct ControllerState
{
	double dX;
	double dY;
	double dZ;
	double dYaw;
	double dPitch;
	double dRoll;
};

struct MouseState
{
	float dX;
	float dY;
};

class ROTTR_Cam
{
public:
	BYTE Pad000[0x38];
	float m_fov;
	BYTE Pad03C[0x24];
	XMMATRIX m_transform1;
	XMMATRIX m_transform2;
};

class Camera
{
public:
	double m_X;
	double m_Y;
	double m_Z;

	double m_pitch;
	double m_yaw;
	double m_roll;

	float m_fov;

	float m_moveSpeed;
	float m_rotationSpeed;
	float m_rollSpeed;

	XMMATRIX m_matrix;
	XMMATRIX m_matrix2;
};

class CameraController
{
public:
	CameraController();

	void ToggleCamera();
	void CameraHook(ROTTR_Cam*);
	void MouseHook(__int64);
	void UpdateCamera(double dt);

	void HotkeyUpdate();

private:
	bool m_enabled;
	Camera m_camera;
	ROTTR_Cam* m_pGameCamera;

	void IssueCommand();

	void ToggleFreeze();
	bool m_timeFrozen;

	bool m_keyboard;
	bool m_controller;
	bool m_inputDisabled;
	void UpdateControls();
	void ToggleGameControls();

	MouseState m_mouseState;
	ControllerState m_controllerState;
	int m_controllerID;
	XINPUT_STATE m_state;
	bool FindController();

	double m_mousePitchHistory[100];
	double m_mouseYawHistory[100];
	double m_mouseRollHistory[100];
};