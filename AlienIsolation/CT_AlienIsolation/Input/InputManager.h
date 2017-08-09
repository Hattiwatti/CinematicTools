#pragma once
#include "../Camera/CameraHelpers.h"
#include <dinput.h>
#include <XInput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "XInput9_1_0.lib")

enum ControllerType
{
  XInput,
  DirectInput
};


class InputManager
{
public:
  InputManager();
  ~InputManager();

  bool GetGamepadState(GamepadState& pState);
  HRESULT CreateDevice(LPCDIDEVICEINSTANCE);

private:
  static void HotkeyThread();
  static void ControllerThread();

  bool FindDInputController();
  bool FindXInputController();

  void ParseDInputData(GamepadState& pState, DIJOYSTATE2& state);
  void ParseXInputData(GamepadState& pState, XINPUT_STATE& state);

  LPDIRECTINPUT8 lpdi;
  LPDIRECTINPUTDEVICE8 lpdiGamepad;

  int m_xinputID;

  XINPUT_STATE m_xiState;
  DIJOYSTATE2 m_diState;

  bool m_hasController;
  ControllerType m_controllerType;

public:
  InputManager(InputManager const&) = delete;
  void operator=(InputManager const&) = delete;
};