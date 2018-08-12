#include "Main.h"
#include "Hotkeys.h"
#include "Hooks.h"
#include "Offsets.h"
#include "Tools\Log\Log.h"

using namespace boost::chrono;

high_resolution_clock Main::Clock;
high_resolution_clock::time_point prevTime;
bool Main::m_exit = false;

CameraController Main::_CameraController;

DWORD WINAPI Main::Init(LPVOID lpargs)
{
	Log::Init(Log::DebugMode::CONSOLE);
	Log::Write("Rise of the Tomb Raider Freecamera tool by Hattiwatti");
	Offsets::Init();
	Hooks::Init();
	
	HANDLE thread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Hotkeys::Init, NULL, NULL, NULL);
	CloseHandle(thread);

	while (!m_exit)
		Update();

	return 1;
}

void Main::Update()
{
	duration<double> dt = Clock.now() - prevTime;
	prevTime = Clock.now();

	_CameraController.UpdateCamera(dt.count());

	Sleep(1);
}
