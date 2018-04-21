#pragma once
#include "ActionDefs.h"
#include "../inih/cpp/INIReader.h"

#include <array>
#include <dinput.h>
#include <Xinput.h>

enum GamepadType
{
  XInput,
  DirectInput
};

struct GamepadInfo
{
  bool IsPresent{ false };
  GamepadType Type;

  int XInputId{ 0 };
  LPDIRECTINPUTDEVICE8 DInputGamepad{ NULL };

  XINPUT_STATE XInputState{ 0 };
  DIJOYSTATE2 DInputState{ 0 };
};

class InputSystem
{
public:
  InputSystem();
  ~InputSystem();

  void Initialize();
  void HandleMouseMsg(LPARAM lParam);

  bool IsActionDown(Action action);
  float GetActionState(Action action);

  void ReadConfig(INIReader* pReader);
  const std::string GetConfig();

private:
  void ActionUpdate();
  void ControllerUpdate();
  void HotkeyUpdate();

  void UpdateXInput();
  void UpdateDInput();

  static BOOL DIEnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);

private:
  LPDIRECTINPUT8 m_DInputInterface;
  GamepadInfo m_Gamepad;

  std::array<int, Action::ActionCount>              m_KeyboardBindings;
  std::array<int, Action::ActionCount>              m_GamepadBindings;

  std::array<float, Action::ActionCount>            m_WantedActionStates;
  std::array<float, Action::ActionCount>            m_SmoothActionStates;
  std::array<float, GamepadKey::GamepadKey_Count>   m_GamepadKeyStates;

  std::pair<float,float> m_MouseState;

public:
  InputSystem(InputSystem const&) = delete;
  void operator=(InputSystem const&) = delete;
};