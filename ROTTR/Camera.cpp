#include "Camera.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <vector>
#include "Offsets.h"
#include "Tools/Log/Log.h"

BOOL WriteMemory(DWORD_PTR dwAddress, const void* cpvPatch, DWORD dwSize)
{
	DWORD dwProtect;
	if (VirtualProtect((void*)dwAddress, dwSize, PAGE_READWRITE, &dwProtect)) //Unprotect the memory
		memcpy((void*)dwAddress, cpvPatch, dwSize); //Write our patch
	else
		return false; //Failed to unprotect, so return false..
	return VirtualProtect((void*)dwAddress, dwSize, dwProtect, new DWORD); //Reprotect the memory
}

CameraController::CameraController()
{
	m_enabled = false;
	m_keyboard = true;
	m_controller = true;
	m_camera.m_moveSpeed = 100.0f;
	m_camera.m_rotationSpeed = 0.4f;
	m_camera.m_rollSpeed = 0.15f;
}

void CameraController::ToggleCamera()
{
	if (!m_enabled)
	{
		m_camera.m_X = m_pGameCamera->m_transform1.r[3].m128_f32[0];
		m_camera.m_Y = m_pGameCamera->m_transform1.r[3].m128_f32[1];
		m_camera.m_Z = m_pGameCamera->m_transform1.r[3].m128_f32[2];
		m_camera.m_fov = m_pGameCamera->m_fov;
		m_camera.m_matrix = XMMatrixIdentity();
		m_camera.m_matrix2 = XMMatrixIdentity();
	}

	m_enabled = !m_enabled;
	Log::Write("Camera enabled: " + string(m_enabled ? "True" : "False"));
}

void CameraController::UpdateCamera(double dt)
{
	if (!m_enabled) return;

	UpdateControls();

	double m_averagePitch = 0;
	double m_averageYaw = 0;
	double m_averageRoll = 0;

	// Smoothen movement by calculating the average delta from 100 previous frames

	for (int i = 0; i<100; i++)
	{
		m_averagePitch += m_mousePitchHistory[i];
		m_averageYaw += m_mouseYawHistory[i];
		m_averageRoll += m_mouseRollHistory[i];
	}

	m_camera.m_pitch = -(m_averagePitch / 100) * dt * m_camera.m_rotationSpeed;
	m_camera.m_yaw = -(m_averageYaw / 100) * dt * m_camera.m_rotationSpeed;
	m_camera.m_roll = (m_averageRoll / 100) * dt * m_camera.m_rollSpeed;

	XMMATRIX camTransform = XMMatrixRotationRollPitchYaw(m_camera.m_pitch, 0, 0);
	camTransform = XMMatrixMultiply(XMMatrixRotationRollPitchYaw(0, m_camera.m_yaw, 0), camTransform);
	camTransform = XMMatrixMultiply(XMMatrixRotationRollPitchYaw(0, 0, m_camera.m_roll), camTransform);

	camTransform = XMMatrixMultiply(camTransform, m_camera.m_matrix2);

	m_camera.m_matrix2 = camTransform;

	float backupFloat = camTransform.r[0].m128_f32[2];
	camTransform.r[0].m128_f32[2] = camTransform.r[0].m128_f32[1];
	camTransform.r[0].m128_f32[1] = backupFloat;

	backupFloat = camTransform.r[1].m128_f32[2];
	camTransform.r[1].m128_f32[2] = camTransform.r[1].m128_f32[1];
	camTransform.r[1].m128_f32[1] = backupFloat;

	backupFloat = camTransform.r[2].m128_f32[2];
	camTransform.r[2].m128_f32[2] = camTransform.r[2].m128_f32[1];
	camTransform.r[2].m128_f32[1] = backupFloat;

	m_camera.m_X += m_controllerState.dX * camTransform.r[0].m128_f32[0] * 0.01 * m_camera.m_moveSpeed;
	m_camera.m_Y += m_controllerState.dX * camTransform.r[0].m128_f32[1] * 0.01 * m_camera.m_moveSpeed;
	m_camera.m_Z += m_controllerState.dX * camTransform.r[0].m128_f32[2] * 0.01 * m_camera.m_moveSpeed;

	m_camera.m_X += m_controllerState.dY * camTransform.r[1].m128_f32[0] * 0.01 * m_camera.m_moveSpeed;
	m_camera.m_Y += m_controllerState.dY * camTransform.r[1].m128_f32[1] * 0.01 * m_camera.m_moveSpeed;
	m_camera.m_Z += m_controllerState.dY * camTransform.r[1].m128_f32[2] * 0.01 * m_camera.m_moveSpeed;

	m_camera.m_X += m_controllerState.dZ * camTransform.r[2].m128_f32[0] * 0.01 * m_camera.m_moveSpeed;
	m_camera.m_Y += m_controllerState.dZ * camTransform.r[2].m128_f32[1] * 0.01 * m_camera.m_moveSpeed;
	m_camera.m_Z += m_controllerState.dZ * camTransform.r[2].m128_f32[2] * 0.01 * m_camera.m_moveSpeed;


	camTransform.r[3] = XMVectorSet(m_camera.m_X, m_camera.m_Y, m_camera.m_Z, 0);
	
	m_camera.m_matrix = camTransform;

	m_controllerState.dPitch = 0;
	m_controllerState.dYaw = 0;
	m_controllerState.dRoll = 0;
	m_controllerState.dZ = 0;
	m_controllerState.dX = 0;
	m_controllerState.dY = 0;
}

void CameraController::CameraHook(ROTTR_Cam* pCamera)
{
	m_pGameCamera = pCamera;
	if (m_enabled)
	{
		m_pGameCamera->m_transform1 = m_camera.m_matrix;
		m_pGameCamera->m_transform2 = m_camera.m_matrix;
		m_pGameCamera->m_fov = m_camera.m_fov;
	}
}

void CameraController::UpdateControls()
{
	for (int i = 99; i > 0; i -= 1)
	{
		m_mousePitchHistory[i] = m_mousePitchHistory[i - 1];
		m_mouseYawHistory[i] = m_mouseYawHistory[i - 1];
		m_mouseRollHistory[i] = m_mouseRollHistory[i - 1];
	}

	m_mousePitchHistory[0] = 0;
	m_mouseYawHistory[0] = 0;
	m_mouseRollHistory[0] = 0;

	if (FindController() && m_controller)
	{
		{
			double LX = m_state.Gamepad.sThumbLX;
			double LY = m_state.Gamepad.sThumbLY;

			//determine how far the controller is pushed
			double magnitude = sqrt(LX*LX + LY*LY);

			//determine the direction the controller is pushed
			double normalizedLX = LX / magnitude;
			double normalizedLY = LY / magnitude;

			double normalizedMagnitude = 0;

			//check if the controller is outside a circular dead zone
			if (magnitude > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			{
				//clip the magnitude at its expected maximum value
				if (magnitude > 32767) magnitude = 32767;

				//adjust magnitude relative to the end of the dead zone
				magnitude -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;

				//optionally normalize the magnitude with respect to its expected range
				//giving a magnitude value of 0.0 to 1.0
				normalizedMagnitude = magnitude / (32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
			}
			else //if the controller is in the deadzone zero out the magnitude
			{
				magnitude = 0.0;
				normalizedMagnitude = 0.0;
			}
			if (magnitude > 0.0) {
				normalizedMagnitude *= normalizedMagnitude;
				// Determine the direction the controller is pushed
				double normalizedLX = LX / magnitude;
				double normalizedLY = LY / magnitude;
				m_controllerState.dX += normalizedLX * normalizedMagnitude;
				m_controllerState.dZ += (normalizedLY * normalizedMagnitude);
			}
			else
			{
				m_controllerState.dX += 0.0;
				m_controllerState.dZ += 0.0;
			}
		}

		{
			// Right thumb
			double RX = m_state.Gamepad.sThumbRX;
			double RY = m_state.Gamepad.sThumbRY;

			// Determine how far the controller is pushed
			double magnitude = sqrt(RX*RX + RY*RY);

			double normalizedMagnitude = 0;

			// Check if the controller is outside a circular dead zone
			if (magnitude > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
			{
				// Clip the magnitude at its expected maximum value
				if (magnitude > 32767) magnitude = 32767;

				// Adjust magnitude relative to the end of the dead zone
				magnitude -= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;

				// Optionally normalize the magnitude with respect to its expected range
				// Giving a magnitude value of 0.0 to 1.0
				normalizedMagnitude = magnitude / (32767 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
			}
			else {
				// If the controller is in the deadzone zero out the magnitude
				magnitude = 0.0;
				normalizedMagnitude = 0.0;
			}

			if (magnitude > 0.0) {
				normalizedMagnitude *= normalizedMagnitude;
				// Determine the direction the controller is pushed
				double normalizedRX = RX / magnitude;
				double normalizedRY = RY / magnitude;
				m_controllerState.dYaw += -(normalizedRX * normalizedMagnitude);
				m_controllerState.dPitch += (normalizedRY * normalizedMagnitude);
			}
			else {
				m_controllerState.dYaw += 0.0;
				m_controllerState.dPitch += 0.0;
			}
		}

		{
			// Triggers
			int delta = -(int)(m_state.Gamepad.bLeftTrigger) + (int)(m_state.Gamepad.bRightTrigger);
			if (delta < XINPUT_GAMEPAD_TRIGGER_THRESHOLD / 2 && delta >(-XINPUT_GAMEPAD_TRIGGER_THRESHOLD / 2)) {
				delta = 0;
			}
			else if (delta < 0) {
				delta += XINPUT_GAMEPAD_TRIGGER_THRESHOLD / 2;
			}
			else {
				delta -= XINPUT_GAMEPAD_TRIGGER_THRESHOLD / 2;
			}
			m_controllerState.dY += ((double)delta) / (255.0 - XINPUT_GAMEPAD_TRIGGER_THRESHOLD / 2);
		}

		if ((m_state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0)
			m_controllerState.dRoll += 1;
		else if ((m_state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0)
			m_controllerState.dRoll += -1;
		else
			m_controllerState.dRoll += 0;

		if ((m_state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0)
			m_camera.m_roll = 0;

		if ((m_state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0)
			m_camera.m_fov += 0.001;
		if ((m_state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0)
			m_camera.m_fov -= 0.001;
	}
	if (m_keyboard)
	{
		if (GetAsyncKeyState(VK_UP) & 0x8000)
			m_controllerState.dPitch += 1;
		if (GetAsyncKeyState(VK_DOWN) & 0x8000)
			m_controllerState.dPitch += -1;
		if (GetAsyncKeyState(VK_LEFT) & 0x8000)
			m_controllerState.dYaw += 1;
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
			m_controllerState.dYaw += -1;
		if (GetAsyncKeyState(VK_NUMPAD1) & 0x8000)
			m_controllerState.dRoll += 1;
		if (GetAsyncKeyState(VK_NUMPAD3) & 0x8000)
			m_controllerState.dRoll += -1;
		if (GetAsyncKeyState(VK_NUMPAD8) & 0x8000)
			m_controllerState.dZ += 1;
		if (GetAsyncKeyState(VK_NUMPAD5) & 0x8000)
			m_controllerState.dZ -= 1;
		if (GetAsyncKeyState(VK_NUMPAD6) & 0x8000)
			m_controllerState.dX += 1;
		if (GetAsyncKeyState(VK_NUMPAD4) & 0x8000)
			m_controllerState.dX -= 1;
		if (GetAsyncKeyState(VK_NUMPAD7) & 0x8000)
			m_controllerState.dY += 1;
		if (GetAsyncKeyState(VK_NUMPAD9) & 0x8000)
			m_controllerState.dY -= 1;
	}
	if (m_inputDisabled)
	{
		//m_controllerState.dYaw += -m_mouseState.dX*100;
		//m_controllerState.dPitch += -m_mouseState.dY * 100;
	}

	m_mouseYawHistory[0] = m_controllerState.dYaw;
	m_mousePitchHistory[0] = m_controllerState.dPitch;
	m_mouseRollHistory[0] = m_controllerState.dRoll;
}

bool CameraController::FindController()
{
	for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
	{
		ZeroMemory(&m_state, sizeof(XINPUT_STATE));
		DWORD dwResult = XInputGetState(i, &m_state);
		if (dwResult == ERROR_SUCCESS)
		{
			m_controllerID = i;
			return true;
		}
	}
	return false;
}

void CameraController::ToggleGameControls()
{
	if (!m_inputDisabled)
	{
		BYTE ret[3] = { 0xC3, 0x90, 0x90 };
		WriteMemory(Offsets::GetOffset("OFFSET_GAMECONTROLS"), ret, 0x3);
	}
	else
	{
		BYTE orig[3] = { 0x4C, 0x8B, 0xDC };
		WriteMemory(Offsets::GetOffset("OFFSET_GAMECONTROLS"), orig, 0x3);
	}
	m_inputDisabled = !m_inputDisabled;
	Log::Write("Input disabled: " + string(m_inputDisabled ? "True" : "False"));
}

void CameraController::MouseHook(__int64 input)
{
	//if (m_inputDisabled && m_enabled)
	//{
	//	m_mouseState.dX = *(float*)(input + 0xC0);
	//	m_mouseState.dY = *(float*)(input + 0xCC);
	//}
}

void CameraController::ToggleFreeze()
{
	if (!m_timeFrozen)
	{
		*((float*)Offsets::GetOffset("OFFSET_TIMESCALE")) = 0;

		BYTE op1[7] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
		WriteMemory(Offsets::GetOffset("OFFSET_FREEZENOP1"), op1, 0x7);

		//BYTE op2[2] = { 0x90, 0x90 };
		//WriteMemory(Offsets::GetOffset("OFFSET_FREEZENOP2"), op2, 0x2);

		BYTE op3[8] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
		WriteMemory(Offsets::GetOffset("OFFSET_FREEZENOP3"), op3, 0x8);

		BYTE op4[6] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
		WriteMemory(Offsets::GetOffset("OFFSET_FREEZENOP4"), op4, 0x6);
	}
	else
	{
		*((float*)Offsets::GetOffset("OFFSET_TIMESCALE")) = 1;

		BYTE op1[7] = { 0x48, 0x89, 0x86, 0x80, 0x02, 0x00, 0x00 };
		WriteMemory(Offsets::GetOffset("OFFSET_FREEZENOP1"), op1, 0x7);

		//BYTE op2[2] = { 0x89, 0x1F };
		//WriteMemory(Offsets::GetOffset("OFFSET_FREEZENOP2"), op2, 0x2);

		BYTE op3[8] = { 0xF3, 0x0F, 0x11, 0x83, 0xA4, 0x00, 0x00, 0x00 };
		WriteMemory(Offsets::GetOffset("OFFSET_FREEZENOP3"), op3, 0x8);

		BYTE op4[6] = { 0x89, 0x83, 0xA8, 0x00, 0x00, 0x00 };
		WriteMemory(Offsets::GetOffset("OFFSET_FREEZENOP4"), op4, 0x6);
	}

	m_timeFrozen = !m_timeFrozen;
	Log::Write("Time frozen: " + string(m_timeFrozen ? "True" : "False"));
}

void CameraController::IssueCommand()
{
	std::string cmd;
	std::cout << "Command: ";
	std::getline(std::cin, cmd);

	std::vector<std::string> strs;
	boost::to_lower(cmd);
	boost::split(strs, cmd, boost::is_any_of(" "));

	if (strs[0] == "speed")
	{
		m_camera.m_moveSpeed = atof(strs[1].c_str());
	}
	else if (strs[0] == "rotspeed")
	{
		m_camera.m_rotationSpeed = atof(strs[1].c_str());
	}
	else if (strs[0] == "rollspeed")
	{
		m_camera.m_rollSpeed = atof(strs[1].c_str());
	}
}