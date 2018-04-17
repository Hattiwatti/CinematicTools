#pragma once
#include "../Util/ActionHelpers.h"
#include "../Inih/cpp/INIReader.h"
#include <map>
#include <dinput.h>
#include <vector>
#include <string>
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

  void Initialize(INIReader*);
  void Release();

  float GetActionState(Action actionId);
  bool IsActionDown(Action actionId);
  std::tuple<int, int> GetMouseState() { return m_mouseState; }

  HRESULT CreateDevice(LPCDIDEVICEINSTANCE);

  void DrawConfigUI();
  std::vector<std::string> GetSettings();

  bool KeyDown(LPARAM lparam, WPARAM wparam);
  bool IsCapturingKey() { return m_captureGamepadKey || m_captureKeyboardKey; }

  void HandleInputMessage(LPARAM lParam);

private:
  static DWORD WINAPI UpdateThread(LPVOID lpArg);
  static DWORD WINAPI HotkeyThread(LPVOID lpArg);
  static DWORD WINAPI ControllerThread(LPVOID lpArg);

  bool FindDInputController();
  bool FindXInputController();

  void ParseDInputData(DIJOYSTATE2& state);
  void ParseXInputData(XINPUT_STATE& state);

  void ReadConfig(INIReader*);

private:
  RAWINPUTDEVICE Rid;
  LPDIRECTINPUT8 lpdi;
  LPDIRECTINPUTDEVICE8 lpdiGamepad;

  bool m_hasController;
  int m_xinputID;
  ControllerType m_controllerType;
  XINPUT_STATE m_xiState;
  DIJOYSTATE2 m_diState;

  int m_keyboardMap[Action::Action_Count];
  GamepadKey m_gamepadMap[Action::Action_Count];

  float m_wantedActionStates[Action::Action_Count];
  float m_smoothActionStates[Action::Action_Count];
  float m_gamepadKeyStates[GamepadKey::GamepadKey_Count];

  std::tuple<int, int> m_mouseState;

  std::map<int, int> m_hotkeyMap;
  std::string* m_sHotkeyMap;
  bool m_captureKeyboardKey;

  bool m_hotkeyWindowFocus;
  std::string m_sPreviousHotkey;
  std::string m_sCapturedHotkey;
  int m_captureIndex;
  int m_capturedHotkey;

  bool m_captureGamepadKey;
  GamepadKey m_previousGamepadKey;

  void StartKeyboardCapture(int index);
  void StartGamepadCapture(int index);

public:
  InputManager(InputManager const&) = delete;
  void operator=(InputManager const&) = delete;
};