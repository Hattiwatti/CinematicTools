#pragma once
#include "ActionDefs.h"
#include "../inih/cpp/INIReader.h"

#include <dinput.h>
#include <Xinput.h>

enum GamepadType
{
  XInput,
  DirectInput
};

struct GamepadInfo
{
  bool IsPresent;
  GamepadType Type;

  int XInputId;
  LPDIRECTINPUTDEVICE8 DInputGamepad;

  XINPUT_STATE XInputState;
  DIJOYSTATE2 DInputState;
};

class InputSystem
{
public:
  InputSystem();
  ~InputSystem();

  void Initialize();
  void HandleMouseMsg();

  void ReadConfig(INIReader* pReader);
  const std::string GetConfig();

private:
  void ActionUpdate();
  void ControllerUpdate();
  void HotkeyUpdate();
  
private:
  LPDIRECTINPUT8 m_DInputInterface;
  GamepadInfo m_Gamepad;

public:
  InputSystem(InputSystem const&) = delete;
  void operator=(InputSystem const&) = delete;
};