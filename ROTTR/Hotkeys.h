#pragma once
#include <Windows.h>
#include "Main.h"

class Hotkeys
{
public:
	static DWORD WINAPI Init(LPVOID lpargs);
	static void Update();
	static void DeInit();

private:
	static bool m_closing;
};