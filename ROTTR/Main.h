#pragma once
#include <Windows.h>
#include "boost\chrono.hpp"
#include "Camera.h"

class Main
{
public:
	static DWORD WINAPI Init(LPVOID lpargs);
	static boost::chrono::high_resolution_clock Clock;

	static CameraController _CameraController;

private:
	static void Update();
	static bool m_exit;
};
