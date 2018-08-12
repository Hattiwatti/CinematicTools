#include "Hooks.h"
#include "Offsets.h"

typedef void(__fastcall* tCameraHook)(__int64, __int64, __int64);
typedef __int64(__fastcall* tMouseHook)(__int64, __int64, char, __int64);

tCameraHook oCameraHook = NULL;
tMouseHook oMouseHook = NULL;

void __fastcall hCameraHook(__int64 a1, __int64 a2, __int64 a3)
{
	oCameraHook(a1, a2, a3);
	Main::_CameraController.CameraHook((ROTTR_Cam*)a1);
}

__int64 __fastcall hMouseHook(__int64 a1, __int64 a2, char a3, __int64 a4)
{
	Main::_CameraController.MouseHook(a4);
	return oMouseHook(a1, a2, a3, a4);
}

void Hooks::Init()
{
	if (MH_Initialize() != MH_OK)
		printf("Couldn't initialize MinHook\n");

	if (MH_CreateHook((PVOID*)Offsets::GetOffset("OFFSET_CAMERAHOOK"), hCameraHook, reinterpret_cast<void**>(&oCameraHook)) != MH_OK)
		printf("Couldn't create Camera hook\n");
	if (MH_EnableHook((PVOID*)Offsets::GetOffset("OFFSET_CAMERAHOOK")) != MH_OK)
		printf("Can't enable Camera hook\n");

	if (MH_CreateHook((PVOID*)Offsets::GetOffset("OFFSET_MOUSEHOOK"), hMouseHook, reinterpret_cast<void**>(&oMouseHook)) != MH_OK)
		printf("Couldn't create Mouse hook\n");
	if (MH_EnableHook((PVOID*)Offsets::GetOffset("OFFSET_MOUSEHOOK")) != MH_OK)
		printf("Can't enable Mouse hook\n");
}