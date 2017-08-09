#include "InputManager.h"
#include "../AlienIsolation.h"
#include "../Main.h"
#include "../Util/Log.h"

BOOL DIEnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);

InputManager::InputManager()
{
  lpdi = nullptr;
  lpdiGamepad = nullptr;

  HRESULT hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8A, (LPVOID*)&lpdi, NULL);
  if (hr != S_OK)
    Log::Error("Unable to create DirectInput interface. Error code 0x%X", hr);

  m_hasController = FindXInputController() || FindDInputController();

  CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)HotkeyThread, NULL, NULL, NULL);
  //CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ControllerThread, NULL, NULL, NULL);
}

bool InputManager::GetGamepadState(GamepadState& input)
{
  memset(&input, 0, sizeof(GamepadState));
  if (!m_hasController)
    return false;

  if (m_controllerType == ControllerType::DirectInput)
  {
    DIJOYSTATE2 diState;
    ZeroMemory(&diState, sizeof(DIJOYSTATE2));

    HRESULT result = lpdiGamepad->GetDeviceState(sizeof(DIJOYSTATE2), &diState);
    if (result != S_OK)
    {
      lpdiGamepad->Release();
      lpdiGamepad = nullptr;
      Log::Warning("DirectInput controller lost");
      m_hasController = false;
      return false;
    }

    ParseDInputData(input, diState);
    return true;
  }
  else
  {
    XINPUT_STATE xiState;
    ZeroMemory(&xiState, sizeof(XINPUT_STATE));

    DWORD dwResult = XInputGetState(m_xinputID, &xiState);
    if (dwResult != ERROR_SUCCESS)
    {
      m_hasController = false;
      Log::Warning("Xbox controller lost");
      return false;
    }

    ParseXInputData(input, xiState);
    return true;
  }

  return false;
}

bool InputManager::FindXInputController()
{
  for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
  {
    ZeroMemory(&m_xiState, sizeof(XINPUT_STATE));
    DWORD dwResult = XInputGetState(i, &m_xiState);
    if (dwResult == ERROR_SUCCESS)
    {
      m_xinputID = i;
      m_controllerType = ControllerType::XInput;
      Log::Write("Found an Xbox controller");
      m_hasController = true;
      return true;
    }
  }
  return false;
}

bool InputManager::FindDInputController()
{
  HRESULT result;
  if (lpdi == NULL)
    return false;

  HRESULT hr = lpdi->EnumDevices(DI8DEVCLASS_GAMECTRL, (LPDIENUMDEVICESCALLBACKW)DIEnumDevicesCallback, this, DIEDFL_ATTACHEDONLY);
  if (hr != S_OK)
  {
    Log::Error("Failed to enumerate DirectInput devices. Error code 0x%X", hr);
    return false;
  }

  if (lpdiGamepad == NULL)
    return false;

  if ((result = lpdiGamepad->SetDataFormat(&c_dfDIJoystick2)) != S_OK)
    return false;
  if ((result = lpdiGamepad->Acquire()) != S_OK)
    return false;

  m_controllerType = ControllerType::DirectInput;
  m_hasController = true;
  Log::Write("Found a DirectInput controller");
  return true;
}

void InputManager::ParseDInputData(GamepadState& input, DIJOYSTATE2& state)
{
  // Left Stick
  {
    double lX = (double)state.lX - 32767;
    double lY = (double)state.lY - 32767;

    double Magnitude = sqrt(lX*lX + lY*lY);
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
      input.leftStick.x += NormalizedLX * NormalizedMagnitude;
      input.leftStick.y += NormalizedLY * NormalizedMagnitude;
    }
  }

  // Right Stick
  {
    double lX = (double)state.lZ - 32767;
    double lY = (double)state.lRz - 32767;

    double Magnitude = sqrt(lX*lX + lY*lY);
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
      input.rightStick.x -= NormalizedLX * NormalizedMagnitude;
      input.rightStick.y -= NormalizedLY * NormalizedMagnitude;
    }
  }

  // Triggers
  {
    int delta = -(int)(state.lRx) + (int)(state.lRy);
    if (delta < 6553 / 2 && delta >(-6553 / 2)) {
      delta = 0;
    }
    else if (delta < 0) {
      delta += 6553 / 2;
    }
    else {
      delta -= 6553 / 2;
    }
    input.trigger += ((double)delta) / (65535.0 - 6553 / 2);
  }

  input.leftShoulder = state.rgbButtons[4] == 128;
  input.rightShoulder = state.rgbButtons[5] == 128;
  input.leftThumbButton = state.rgbButtons[10] == 128;
  input.rightThumbButton = state.rgbButtons[11] == 128;
  input.dpad_Up = state.rgdwPOV[0] == 0;
  input.dpad_Down = state.rgdwPOV[0] == 18000;
  input.dpad_Left = state.rgdwPOV[0] == 27000;
  input.dpad_Right = state.rgdwPOV[0] == 9000;

  for (int i = 0; i < 4; ++i)
    input.Buttons[i] = state.rgbButtons[i] == 128;
}

void InputManager::ParseXInputData(GamepadState& input, XINPUT_STATE& state)
{
  {
    double LX = state.Gamepad.sThumbLX;
    double LY = state.Gamepad.sThumbLY;

    //determine how far the controller is pushed
    double magnitude = sqrt(LX*LX + LY*LY);

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
      double normalizedLX = LX / magnitude;
      double normalizedLY = LY / magnitude;
      input.leftStick.x += normalizedLX * normalizedMagnitude;
      input.leftStick.y -= normalizedLY * normalizedMagnitude;
    }
  }

  {
    // Right thumb
    double RX = state.Gamepad.sThumbRX;
    double RY = state.Gamepad.sThumbRY;

    // Determine how far the controller is pushed
    double magnitude = sqrt(RX*RX + RY*RY);

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

    if (magnitude > 0.0) {
      normalizedMagnitude *= normalizedMagnitude;
      // Determine the direction the controller is pushed
      double normalizedRX = RX / magnitude;
      double normalizedRY = RY / magnitude;
      input.rightStick.x -= normalizedRX * normalizedMagnitude;
      input.rightStick.y += normalizedRY * normalizedMagnitude;
    }
  }

  {
    // Triggers
    int delta = -(int)(state.Gamepad.bLeftTrigger) + (int)(state.Gamepad.bRightTrigger);
    if (delta < XINPUT_GAMEPAD_TRIGGER_THRESHOLD / 2 && delta >(-XINPUT_GAMEPAD_TRIGGER_THRESHOLD / 2)) {
      delta = 0;
    }
    else if (delta < 0) {
      delta += XINPUT_GAMEPAD_TRIGGER_THRESHOLD / 2;
    }
    else {
      delta -= XINPUT_GAMEPAD_TRIGGER_THRESHOLD / 2;
    }
    input.trigger += ((double)delta) / (255.0 - XINPUT_GAMEPAD_TRIGGER_THRESHOLD / 2);
  }

  input.leftShoulder = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
  input.rightShoulder = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
  input.leftThumbButton = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
  input.rightThumbButton = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);

  input.dpad_Left = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
  input.dpad_Right = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
  input.dpad_Up = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP);
  input.dpad_Down = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN);

  input.Buttons[0] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_X);
  input.Buttons[1] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A);
  input.Buttons[2] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_B);
  input.Buttons[3] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y);
}

void InputManager::HotkeyThread()
{
  Sleep(1000);

  while (!g_mainHandle->IsExiting())
  {
    if (AI::Rendering::HasFocus())
    {
      //Main::GetCameraManager()->HotkeyUpdate();
      //Main::GetEffectManager()->HotkeyUpdate();
     // Main::GetChromakeyTool()->HotkeyUpdate();
    }
    Sleep(1);
  }
}

void InputManager::ControllerThread()
{
  Sleep(1000);

  while (!g_mainHandle->IsExiting())
  {
    Sleep(500);

    if (!g_mainHandle->GetInputManager()->m_hasController)
      continue;

    g_mainHandle->GetInputManager()->FindDInputController();
    g_mainHandle->GetInputManager()->FindXInputController();
  }
}

BOOL DIEnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
  InputManager* inputManager = static_cast<InputManager*>(pvRef);

  HRESULT hr = inputManager->CreateDevice(lpddi);
  if (FAILED(hr))
  {
    Log::Error("Failed to create DirectInput device. Error code 0x%X", hr);
    return DIENUM_CONTINUE;
  }

  return DIENUM_STOP;
}

HRESULT InputManager::CreateDevice(LPCDIDEVICEINSTANCE lpddi)
{
  if (!lpdi) return 1;
  return lpdi->CreateDevice(lpddi->guidInstance, &lpdiGamepad, NULL);
}

InputManager::~InputManager()
{
  lpdiGamepad->Release();
  lpdi->Release();
}
