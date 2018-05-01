#pragma once
#include "ActionDefs.h"
#include "../inih/cpp/INIReader.h"

#include <array>
#include <dinput.h>
#include <DirectXMath.h>
#include <thread>
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

struct CaptureInfo
{
  bool CaptureKb{ false };
  bool CaptureGamepad{ false };
  int ActionIndex{ 0 };

  int CapturedKbKey{ 0 };
  std::string CapturedKbName{ "" };
};

class InputSystem
{
public:
  InputSystem();
  ~InputSystem();

  void Initialize();
  void HandleMouseMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
  void HandleRawInput(LPARAM lParam);
  bool HandleKeyMsg(WPARAM wParam, LPARAM lParam);

  void ShowUI();
  void DrawUI();

  bool IsActionDown(Action action);
  float GetActionState(Action action);
  DirectX::XMFLOAT3 GetMouseState();

  void ReadConfig(INIReader* pReader);
  const std::string GetConfig();

private:
  void ActionUpdate();
  void ControllerUpdate();
  void HotkeyUpdate();

  void UpdateXInput();
  void UpdateDInput();

  void StartKeyboardCapture(int index);
  void StartGamepadCapture(int index);

  static BOOL DIEnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);

private:
  bool m_ShowUI;

  RAWINPUTDEVICE m_RawInput;
  LPDIRECTINPUT8 m_DInputInterface;
  GamepadInfo m_Gamepad;

  std::array<int, Action::ActionCount>              m_KeyboardBindings;
  std::array<GamepadKey, Action::ActionCount>       m_GamepadBindings;
  std::array<std::string, Action::ActionCount>      m_KeyboardKeyNames;

  std::array<float, Action::ActionCount>            m_WantedActionStates;
  std::array<float, Action::ActionCount>            m_SmoothActionStates;
  std::array<float, GamepadKey::GamepadKey_Count>   m_GamepadKeyStates;

  DirectX::XMFLOAT3 m_MouseState;
  LPDIRECTINPUTDEVICE8 m_DIMouse;

  CaptureInfo m_CaptureState;

  std::thread m_ActionThread;
  std::thread m_ControllerThread;
  std::thread m_HotkeyThread;

public:
  InputSystem(InputSystem const&) = delete;
  void operator=(InputSystem const&) = delete;
};