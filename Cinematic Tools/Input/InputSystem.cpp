#include "InputSystem.h"
#include "../Main.h"
#include "../Util/Util.h"
#include <boost/chrono.hpp>
#include <thread>

// How long it takes for state of action to go from 1 to 0
static const float g_actionClearTime = 0.2f;
static const float g_mouseSensitivity = 1.0f;

InputSystem::InputSystem() :
  m_DInputInterface(NULL),
  m_WantedActionStates(),
  m_SmoothActionStates(),
  m_GamepadKeyStates(),
  m_MouseState()
{

}

InputSystem::~InputSystem()
{

}

void InputSystem::Initialize()
{
  HRESULT hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8A, (LPVOID*)&m_DInputInterface, NULL);
  if (FAILED(hr))
  {
    util::log::Error("Unable to create DirectInput interface. HRESULT 0x%X", hr);
    util::log::Warning("DirectInput controllers unavailable");
  }

  // Create input-related threads
  std::thread actionThread(&InputSystem::ActionUpdate, this);
  std::thread controllerThread(&InputSystem::ControllerUpdate, this);
  std::thread hotkeyThread(&InputSystem::HotkeyUpdate, this);
  actionThread.detach();
  controllerThread.detach();
  hotkeyThread.detach();
}

void InputSystem::HandleMouseMsg(LPARAM lParam)
{
  RECT windowRect;
  if (!GetClientRect(g_gameHwnd, &windowRect))
    return;

  signed short xPosRelative = (lParam & 0xFFFF);
  signed short yPosRelative = (lParam >> 16);

  int deltaX = xPosRelative - (windowRect.right - windowRect.left) / 2;
  int deltaY = yPosRelative - (windowRect.bottom - windowRect.top) / 2;

  m_MouseState = std::pair<float, float>(deltaX * g_mouseSensitivity, deltaY * g_mouseSensitivity);
}

bool InputSystem::IsActionDown(Action action)
{
  return m_WantedActionStates[action] != 0.f;
}

float InputSystem::GetActionState(Action action)
{
  return m_SmoothActionStates[action];
}

void InputSystem::ReadConfig(INIReader* pReader)
{
  for (int i = 0; i < Action::ActionCount; ++i)
  {
    Action action = static_cast<Action>(i);
    std::string name = ActionStringMap.at(action);

    int vkey = pReader->GetInteger("KeyboardMap", name, DefaultKeyboardMap.at(action));
    GamepadKey padKey = static_cast<GamepadKey>(pReader->GetInteger("GamepadMap", name, DefaultGamepadMap.at(action)));

    m_KeyboardBindings[action] = vkey;
    m_GamepadBindings[action] = padKey;
  }
}

void InputSystem::ActionUpdate()
{
  // Action Update thread
  // Processes keyboard + gamepad input and updates
  // action states based on bindings.

  auto lastUpdate = boost::chrono::high_resolution_clock::now();

  while (!g_shutdown)
  {
    boost::chrono::duration<double> dt = boost::chrono::high_resolution_clock::now() - lastUpdate;
    lastUpdate = boost::chrono::high_resolution_clock::now();

    std::array<float, Action::ActionCount> newWantedStates{ 0 };
    m_GamepadKeyStates.fill(0.f);
    
    if (g_hasFocus)
    {
      if (m_Gamepad.IsPresent)
      {
        if (m_Gamepad.Type == GamepadType::XInput)
          UpdateXInput();
        else
          UpdateDInput();

        if (g_mainHandle->GetCameraManager()->IsGamepadDisabled())
        {
          for (int i = 0; i < Action::ActionCount; ++i)
          {
            GamepadKey key = m_GamepadBindings[i];
            newWantedStates[i] += m_GamepadKeyStates[key];
          }
        }
      }

      if (!g_mainHandle->GetUI()->HasKeyboardFocus())
      {
        for (int i = 0; i < Action::ActionCount; ++i)
        {
          int key = m_KeyboardBindings[i];

          //if (!g_mainHandle->GetCameraManager()->IsKbmDisabled() &&
          //  i >= Camera_Forward && i <= Camera_Down)
          //  key = pInputMgr->m_keyboardMap[i + 14];

          if (key >> 8)
          {
            if (GetKeyState(key >> 8) & 0x8000 && GetKeyState(key & 0xFF) & 0x8000)
              newWantedStates[i] += 1.0f;
          }
          else if (GetKeyState(key) & 0x8000)
            newWantedStates[i] += 1.0f;
        }
      }
    }


    // Copy new values and perform smoothing
    m_WantedActionStates = newWantedStates;
    for (int i = 0; i < Action::ActionCount; ++i)
    {
      float& currentState = m_SmoothActionStates[i];
      float& wantedState = m_WantedActionStates[i];

      if (currentState != wantedState)
      {
        if (wantedState > 0)
        {
          currentState += dt.count() / g_actionClearTime;
          if (currentState > wantedState)
            currentState = wantedState;
        }
        else
        {
          currentState -= dt.count() / g_actionClearTime;
          if (currentState < 0)
            currentState = 0;
        }
      }
    }

    Sleep(1);
  }
}

void InputSystem::ControllerUpdate()
{
  // Gamepad detection
  // For some people gamepad scanning seemed to block
  // the update/input threads, so that's why it's placed
  // in its own thread.

  while (!g_shutdown)
  {
    Sleep(1000);
    if (!m_Gamepad.IsPresent)
    {
      // Scan for XInput controllers first so they don't get
      // detected as DInput controllers.
      for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
      {
        DWORD dwResult = XInputGetState(i, &m_Gamepad.XInputState);
        if (dwResult == ERROR_SUCCESS)
        {
          m_Gamepad.XInputId = i;
          m_Gamepad.Type = GamepadType::XInput;
          util::log::Write("Found a XInput controller");
          m_Gamepad.IsPresent = true;
          break;
        }
      }

      if (m_Gamepad.IsPresent) continue;

      // And then lets search for DInput controllers
      if (m_DInputInterface == NULL) continue;
      HRESULT hr = m_DInputInterface->EnumDevices(DI8DEVCLASS_GAMECTRL, DIEnumDevicesCallback, this, DIEDFL_ATTACHEDONLY);
      if (FAILED(hr)) continue;

      if (m_Gamepad.DInputGamepad == NULL) continue;
      hr = m_Gamepad.DInputGamepad->SetDataFormat(&c_dfDIJoystick2);
      if (FAILED(hr)) continue;
      hr = m_Gamepad.DInputGamepad->Acquire();
      if (FAILED(hr)) continue;

      m_Gamepad.IsPresent = true;
      m_Gamepad.Type = GamepadType::DirectInput;
      util::log::Write("Found a DirectInput controller");
    }
  }
}

void InputSystem::HotkeyUpdate()
{
  // Hotkey thread
  // This is in its own thread so when you press and hold
  // for example HUD Toggle, you can still move the camera
  // and it doesn't block other updates until you lift the
  // key up again.

  while (!g_shutdown)
  {
    if (g_hasFocus)
    {

      g_mainHandle->GetCameraManager()->HotkeyUpdate();
    }
    Sleep(1);
  }
}

void InputSystem::UpdateXInput()
{
  XINPUT_STATE xiState{ 0 };
  DWORD dwResult = XInputGetState(m_Gamepad.XInputId, &xiState);
  if (dwResult != ERROR_SUCCESS)
  {
    m_Gamepad.IsPresent = false;
    util::log::Warning("Xbox controller lost");
    return;
  }

  {
    // Left thumb
    double LX = xiState.Gamepad.sThumbLX;
    double LY = xiState.Gamepad.sThumbLY;

    //determine how far the controller is pushed
    double magnitude = sqrt(LX*LX + LY * LY);

    //determine the direction the controller is pushed
    double normalizedLX = LX / magnitude;
    double normalizedLY = LY / magnitude;

    double normalizedMagnitude = 0;

    //check if the controller is outside a circular dead zone
    if (magnitude > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
    {
      //clip the magnitude at its expected maximum value
      if (magnitude > 32767) magnitude = 32767;

      //adjust magnitude relative to the end of the dead zone
      magnitude -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;

      //optionally normalize the magnitude with respect to its expected range
      //giving a magnitude value of 0.0 to 1.0
      normalizedMagnitude = magnitude / (32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    }
    else //if the controller is in the deadzone zero out the magnitude
    {
      magnitude = 0.0;
      normalizedMagnitude = 0.0;
    }
    if (magnitude > 0.0) {
      normalizedMagnitude *= normalizedMagnitude;
      // Determine the direction the controller is pushed
      double NormalizedLX = LX / magnitude;
      double NormalizedLY = LY / magnitude;

      if (NormalizedLX > 0)
        m_GamepadKeyStates[GamepadKey::LeftThumb_XPos] = NormalizedLX * normalizedMagnitude;
      else
        m_GamepadKeyStates[GamepadKey::LeftThumb_XNeg] = -NormalizedLX * normalizedMagnitude;

      if (NormalizedLY > 0)
        m_GamepadKeyStates[GamepadKey::LeftThumb_YPos] = NormalizedLY * normalizedMagnitude;
      else
        m_GamepadKeyStates[GamepadKey::LeftThumb_YNeg] = -NormalizedLY * normalizedMagnitude;
    }
  }

  {
    // Right thumb
    double RX = xiState.Gamepad.sThumbRX;
    double RY = xiState.Gamepad.sThumbRY;

    // Determine how far the controller is pushed
    double magnitude = sqrt(RX*RX + RY * RY);

    double normalizedMagnitude = 0;

    // Check if the controller is outside a circular dead zone
    if (magnitude > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
    {
      // Clip the magnitude at its expected maximum value
      if (magnitude > 32767) magnitude = 32767;

      // Adjust magnitude relative to the end of the dead zone
      magnitude -= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;

      // Optionally normalize the magnitude with respect to its expected range
      // Giving a magnitude value of 0.0 to 1.0
      normalizedMagnitude = magnitude / (32767 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    }
    else {
      // If the controller is in the deadzone zero out the magnitude
      magnitude = 0.0;
      normalizedMagnitude = 0.0;
    }

    if (magnitude > 0.0)
    {
      normalizedMagnitude *= normalizedMagnitude;
      // Determine the direction the controller is pushed
      double NormalizedRX = RX / magnitude;
      double NormalizedRY = RY / magnitude;

      if (NormalizedRX > 0)
        m_GamepadKeyStates[GamepadKey::RightThumb_XPos] = NormalizedRX * normalizedMagnitude;
      else
        m_GamepadKeyStates[GamepadKey::RightThumb_XNeg] = -NormalizedRX * normalizedMagnitude;

      if (NormalizedRY > 0)
        m_GamepadKeyStates[GamepadKey::RightThumb_YPos] = NormalizedRY * normalizedMagnitude;
      else
        m_GamepadKeyStates[GamepadKey::RightThumb_YNeg] = -NormalizedRY * normalizedMagnitude;
    }
  }

  {
    // Triggers
    int delta = -(int)(xiState.Gamepad.bLeftTrigger) + (int)(xiState.Gamepad.bRightTrigger);
    if (delta < XINPUT_GAMEPAD_TRIGGER_THRESHOLD / 2 && delta >(-XINPUT_GAMEPAD_TRIGGER_THRESHOLD / 2)) {
      delta = 0;
    }
    else if (delta < 0) {
      delta += XINPUT_GAMEPAD_TRIGGER_THRESHOLD / 2;
    }
    else {
      delta -= XINPUT_GAMEPAD_TRIGGER_THRESHOLD / 2;
    }
    float trigger = ((double)delta) / (255.0 - XINPUT_GAMEPAD_TRIGGER_THRESHOLD / 2);
    if (trigger > 0)
      m_GamepadKeyStates[GamepadKey::RightTrigger] = trigger;
    else
      m_GamepadKeyStates[GamepadKey::LeftTrigger] = -trigger;
  }

  m_GamepadKeyStates[GamepadKey::LeftThumb] = (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) ? 1.0f : 0.f;
  m_GamepadKeyStates[GamepadKey::RightThumb] = (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) ? 1.0f : 0.f;
  m_GamepadKeyStates[GamepadKey::LeftShoulder] = (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? 1.0f : 0.f;
  m_GamepadKeyStates[GamepadKey::RightShoulder] = (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? 1.0f : 0.f;

  m_GamepadKeyStates[GamepadKey::DPad_Left] = (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) ? 1.0f : 0.f;
  m_GamepadKeyStates[GamepadKey::DPad_Right] = (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) ? 1.0f : 0.f;
  m_GamepadKeyStates[GamepadKey::DPad_Up] = (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) ? 1.0f : 0.f;
  m_GamepadKeyStates[GamepadKey::DPad_Down] = (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) ? 1.0f : 0.f;

  m_GamepadKeyStates[GamepadKey::Button1] = (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_X) ? 1.0f : 0.f;
  m_GamepadKeyStates[GamepadKey::Button2] = (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_A) ? 1.0f : 0.f;
  m_GamepadKeyStates[GamepadKey::Button3] = (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_B) ? 1.0f : 0.f;
  m_GamepadKeyStates[GamepadKey::Button4] = (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_Y) ? 1.0f : 0.f;
  m_GamepadKeyStates[GamepadKey::Button5] = (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) ? 1.0f : 0.f;
  m_GamepadKeyStates[GamepadKey::Button6] = (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_START) ? 1.0f : 0.f;
}

void InputSystem::UpdateDInput()
{
  DIJOYSTATE2 diState{ 0 };
  HRESULT result = m_Gamepad.DInputGamepad->GetDeviceState(sizeof(DIJOYSTATE2), &diState);
  if (result != S_OK)
  {
    m_Gamepad.DInputGamepad->Release();
    m_Gamepad.DInputGamepad = nullptr;
    util::log::Warning("DirectInput controller lost");
    m_Gamepad.IsPresent = false;
    return;
  }

  // Left Stick
  {
    double lX = (double)diState.lX - 32767;
    double lY = (double)diState.lY - 32767;

    double Magnitude = sqrt(lX*lX + lY * lY);
    double NormalizedMagnitude = 0;

    if (Magnitude > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
    {
      if (Magnitude > 32767) Magnitude = 32767;
      Magnitude -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
      NormalizedMagnitude = Magnitude / (32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    }
    else
    {
      Magnitude = 0;
      NormalizedMagnitude = 0;
    }

    if (Magnitude > 0.0)
    {
      NormalizedMagnitude *= NormalizedMagnitude;
      double NormalizedLX = lX / Magnitude;
      double NormalizedLY = lY / Magnitude;

      if (NormalizedLX > 0)
        m_GamepadKeyStates[GamepadKey::LeftThumb_XPos] = NormalizedLX * NormalizedMagnitude;
      else
        m_GamepadKeyStates[GamepadKey::LeftThumb_XNeg] = -NormalizedLX * NormalizedMagnitude;

      if (NormalizedLY < 0)
        m_GamepadKeyStates[GamepadKey::LeftThumb_YPos] = -NormalizedLY * NormalizedMagnitude;
      else
        m_GamepadKeyStates[GamepadKey::LeftThumb_YNeg] = NormalizedLY * NormalizedMagnitude;
    }
  }

  // Right Stick
  {
    double lX = (double)diState.lZ - 32767;
    double lY = (double)diState.lRz - 32767;

    double Magnitude = sqrt(lX*lX + lY * lY);
    double NormalizedMagnitude = 0;

    if (Magnitude > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
    {
      if (Magnitude > 32767) Magnitude = 32767;
      Magnitude -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
      NormalizedMagnitude = Magnitude / (32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    }
    else
    {
      Magnitude = 0;
      NormalizedMagnitude = 0;
    }

    if (Magnitude > 0.0)
    {
      NormalizedMagnitude *= NormalizedMagnitude;
      double NormalizedLX = lX / Magnitude;
      double NormalizedLY = lY / Magnitude;

      if (NormalizedLX > 0)
        m_GamepadKeyStates[GamepadKey::RightThumb_XPos] = NormalizedLX * NormalizedMagnitude;
      else
        m_GamepadKeyStates[GamepadKey::RightThumb_XNeg] = -NormalizedLX * NormalizedMagnitude;

      if (NormalizedLY < 0)
        m_GamepadKeyStates[GamepadKey::RightThumb_YPos] = -NormalizedLY * NormalizedMagnitude;
      else
        m_GamepadKeyStates[GamepadKey::RightThumb_YNeg] = NormalizedLY * NormalizedMagnitude;

    }
  }

  // Triggers
  {
    int delta = -(int)(diState.lRx) + (int)(diState.lRy);
    if (delta < 6553 / 2 && delta >(-6553 / 2)) {
      delta = 0;
    }
    else if (delta < 0) {
      delta += 6553 / 2;
    }
    else {
      delta -= 6553 / 2;
    }

    double trigger = ((double)delta) / (65535.0 - 6553 / 2);
    if (trigger > 0)
      m_GamepadKeyStates[GamepadKey::LeftTrigger] = trigger;
    else
      m_GamepadKeyStates[GamepadKey::RightTrigger] = -trigger;
  }

  m_GamepadKeyStates[GamepadKey::LeftThumb] = (diState.rgbButtons[10] == 128) ? 1.0f : 0.f;
  m_GamepadKeyStates[GamepadKey::RightThumb] = (diState.rgbButtons[11] == 128) ? 1.0f : 0.f;
  m_GamepadKeyStates[GamepadKey::LeftShoulder] = (diState.rgbButtons[4] == 128) ? 1.0f : 0.f;
  m_GamepadKeyStates[GamepadKey::RightShoulder] = (diState.rgbButtons[5] == 128) ? 1.0f : 0.f;

  m_GamepadKeyStates[GamepadKey::DPad_Left] = (diState.rgdwPOV[0] == 27000) ? 1.0f : 0.f;
  m_GamepadKeyStates[GamepadKey::DPad_Right] = (diState.rgdwPOV[0] == 9000) ? 1.0f : 0.f;
  m_GamepadKeyStates[GamepadKey::DPad_Up] = (diState.rgdwPOV[0] == 0) ? 1.0f : 0.f;
  m_GamepadKeyStates[GamepadKey::DPad_Down] = (diState.rgdwPOV[0] == 18000) ? 1.0f : 0.f;

  m_GamepadKeyStates[GamepadKey::Button1] = (diState.rgbButtons[0] == 128) ? 1.0f : 0.f;
  m_GamepadKeyStates[GamepadKey::Button2] = (diState.rgbButtons[1] == 128) ? 1.0f : 0.f;
  m_GamepadKeyStates[GamepadKey::Button3] = (diState.rgbButtons[2] == 128) ? 1.0f : 0.f;
  m_GamepadKeyStates[GamepadKey::Button4] = (diState.rgbButtons[3] == 128) ? 1.0f : 0.f;
  m_GamepadKeyStates[GamepadKey::Button5] = (diState.rgbButtons[8] == 128) ? 1.0f : 0.f;
  m_GamepadKeyStates[GamepadKey::Button6] = (diState.rgbButtons[9] == 128) ? 1.0f : 0.f;
}

BOOL InputSystem::DIEnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
  InputSystem* pInputSystem = static_cast<InputSystem*>(pvRef);

  HRESULT hr = pInputSystem->m_DInputInterface->CreateDevice(lpddi->guidInstance, &pInputSystem->m_Gamepad.DInputGamepad, NULL);
  if (FAILED(hr))
  {
    util::log::Error("Failed to create DirectInput device. HRESULT 0x%X", hr);
    return DIENUM_CONTINUE;
  }

  return DIENUM_STOP;
}
