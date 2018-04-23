#pragma once
#include <boost/chrono.hpp>
#include <dinput.h>
#include <map>
#include <Xinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "XInput9_1_0.lib")

class InputManager
{
public:
  struct GamepadState
  {
    float leftX;
    float leftY;
    float rightX;
    float rightY;
    float trigger;
    BYTE buttons[12];
  };

  enum GamepadKey
  {
    LeftThumb_XPos,
    LeftThumb_XNeg,
    LeftThumb_YPos,
    LeftThumb_YNeg,
    RightThumb_XPos,
    RightThumb_XNeg,
    RightThumb_YPos,
    RightThumb_YNeg,
    LeftTrigger,
    RightTrigger,
    LeftThumb,
    RightThumb,
    LeftShoulder,
    RightShoulder,
    DPad_Left,
    DPad_Right,
    DPad_Up,
    DPad_Down
  };

  enum Action
  {
    Camera_Toggle,
    Camera_FreezeTime,
    Camera_ToggleUI,
    Camera_Forward,
    Camera_Backward,
    Camera_Left,
    Camera_Right,
    Camera_Up,
    Camera_Down,
    Camera_YawLeft,
    Camera_YawRight,
    Camera_PitchUp,
    Camera_PitchDown,
    Camera_RollLeft,
    Camera_RollRight,
    Camera_IncFov,
    Camera_DecFov,
    Track_CreateNode,
    Track_DeleteNode,
    Track_Play,
    UI_Toggle
  };

  enum ControllerType
  {
    XInput,
    DirectInput
  };

  InputManager();
  ~InputManager();

  void Initialize();
  void Release();
  void DrawDebug();

  bool IsKeyDown(Action const&);
  float GetActionState(Action const&);

  HRESULT CreateDevice(LPCDIDEVICEINSTANCE);

private:
  static void ControllerThread();
  static void UpdateThread();

  bool FindDInputController();
  bool FindXInputController();

  void ParseDInputData(DIJOYSTATE2& state);
  void ParseXInputData(XINPUT_STATE& state);

private:
  static const int GAMEPAD_KEY_COUNT = 18;
  static const int ACTION_COUNT = 21;

  LPDIRECTINPUT8 lpdi;
  LPDIRECTINPUTDEVICE8 lpdiGamepad;

  bool m_hasController;
  ControllerType m_controllerType;

  unsigned int m_xinputID;
  XINPUT_STATE m_xiState;
  DIJOYSTATE2 m_diState;

  std::map<Action, int> m_actionMap;
  std::map<Action, GamepadKey> m_gamepadActionMap;

  boost::chrono::high_resolution_clock::time_point m_lastUpdate;
  float m_wantedActionStates[ACTION_COUNT];
  float m_smoothedActionStates[ACTION_COUNT];

  float m_gamepadKeyState[GAMEPAD_KEY_COUNT];
  float m_gamepadKeyStateBackup[GAMEPAD_KEY_COUNT];

  HANDLE m_hControllerThread;
  HANDLE m_hUpdateThread;

public:
  InputManager(InputManager const&) = delete;
  void operator=(InputManager const&) = delete;
};