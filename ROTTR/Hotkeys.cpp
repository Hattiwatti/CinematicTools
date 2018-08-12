#include "Hotkeys.h"
#include "Camera.h"

bool Hotkeys::m_closing;

DWORD WINAPI Hotkeys::Init(LPVOID lpargs)
{
	Sleep(100);
	m_closing = false;
	while (!m_closing)
	{
		Update();
		Sleep(1);
	}

	return 0;
}

void Hotkeys::Update()
{
	Main::_CameraController.HotkeyUpdate();
}

void CameraController::HotkeyUpdate()
{
	if (GetAsyncKeyState(VK_INSERT) & 0x8000)
	{
		ToggleCamera();

		while (GetAsyncKeyState(VK_INSERT) & 0x8000)
			Sleep(1);
	}
	if (GetAsyncKeyState(VK_DELETE) & 0x8000)
	{
		ToggleGameControls();

		while (GetAsyncKeyState(VK_DELETE) & 0x8000)
			Sleep(1);
	}
	if (GetAsyncKeyState(VK_HOME) & 0x8000)
	{
		IssueCommand();

		while (GetAsyncKeyState(VK_HOME) & 0x8000)
			Sleep(1);
	}
	if (GetAsyncKeyState(VK_END) & 0x8000)
	{
		ToggleFreeze();

		while (GetAsyncKeyState(VK_END) & 0x8000)
			Sleep(1);
	}
	if (GetAsyncKeyState(VK_NEXT) & 0x8000)
		m_camera.m_fov += 0.001;
	if (GetAsyncKeyState(VK_PRIOR) & 0x8000)
		m_camera.m_fov -= 0.001;
}

void Hotkeys::DeInit()
{
	m_closing = true;
}