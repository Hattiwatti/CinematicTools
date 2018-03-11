#include "InputManager.h"
#include "../Main.h"
#include "../Util/Util.h"

BOOL DIEnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);

InputManager::InputManager()
{
  m_hasController = false;
  m_xinputID = 0;
  lpdi = nullptr;
  lpdiGamepad = nullptr;
  m_hControllerThread = NULL;
  ZeroMemory(m_wantedActionStates, sizeof(m_wantedActionStates));
  ZeroMemory(m_smoothedActionStates, sizeof(m_smoothedActionStates));
  ZeroMemory(m_gamepadKeyState, sizeof(m_gamepadKeyState));
}

InputManager::~InputManager()
{

}

void InputManager::Initialize()
{
  HRESULT hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8A, (LPVOID*)&lpdi, NULL);
  if (hr != S_OK)
    util::log::Error("Unable to create DirectInput interface. HRESULT 0x%X", hr);

  m_actionMap = {
    { Camera_Toggle, VK_INSERT },
    { Camera_ToggleUI, VK_HOME },
    { Camera_FreezeTime, VK_DELETE },
    { Camera_Forward, 'W'},
    { Camera_Backward, 'S'},
    { Camera_Left, 'A'},
    { Camera_Right, 'D'},
    { Camera_Up, VK_SPACE },
    { Camera_Down, VK_LCONTROL },
    { Camera_YawLeft, VK_LEFT },
    { Camera_YawRight, VK_RIGHT },
    { Camera_PitchUp, VK_UP },
    { Camera_PitchDown, VK_DOWN },
    { Camera_RollLeft, VK_NUMPAD1 },
    { Camera_RollRight, VK_NUMPAD3 },
    { Camera_IncFov, VK_PRIOR },
    { Camera_DecFov, VK_NEXT },
    { Track_CreateNode, 'F'},
    { Track_DeleteNode, 'G' },
    { Track_Play, 'P' },
    { UI_Toggle, VK_F5 }
  };

  m_gamepadActionMap = {
    { Camera_Forward, GamepadKey::LeftThumb_YPos },
    { Camera_Backward, GamepadKey::LeftThumb_YNeg },
    { Camera_Left, GamepadKey::LeftThumb_XNeg },
    { Camera_Right, GamepadKey::LeftThumb_XPos },
    { Camera_Up, GamepadKey::LeftTrigger },
    { Camera_Down, GamepadKey::RightTrigger },
    { Camera_YawLeft, GamepadKey::RightThumb_XNeg },
    { Camera_YawRight, GamepadKey::RightThumb_XPos },
    { Camera_PitchUp, GamepadKey::RightThumb_YPos },
    { Camera_PitchDown, GamepadKey::RightThumb_YNeg },
    { Camera_RollLeft, GamepadKey::LeftShoulder },
    { Camera_RollRight, GamepadKey::RightShoulder },
    { Camera_IncFov, GamepadKey::LeftThumb },
    { Camera_DecFov, GamepadKey::RightThumb }
  };

  m_hControllerThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ControllerThread, 0, 0, 0);
  m_hUpdateThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)UpdateThread, 0, 0, 0);

  m_lastUpdate = boost::chrono::high_resolution_clock::now();
}

void InputManager::Release()
{
  if (lpdiGamepad)
    lpdiGamepad->Release();
  if (lpdi)
    lpdi->Release();

  if (m_hControllerThread)
    TerminateThread(m_hControllerThread, 0);
  if (m_hUpdateThread)
    TerminateThread(m_hUpdateThread, 0);
}

void InputManager::UpdateThread()
{
  InputManager* _this = g_mainHandle->GetInputManager();
  while (!g_shutdown)
  {
    boost::chrono::duration<double> dt = boost::chrono::high_resolution_clock::now() - _this->m_lastUpdate;
    _this->m_lastUpdate = boost::chrono::high_resolution_clock::now();

    float newWantedStates[ACTION_COUNT];

    ZeroMemory(newWantedStates, sizeof(newWantedStates));
    ZeroMemory(_this->m_gamepadKeyState, sizeof(_this->m_gamepadKeyState));

    if (TD::RogueClient::Singleton()->m_isWindowFocused)
    {
      if (!g_mainHandle->GetUIManager()->HasKeyboardFocus())
      {
        for (auto itr = _this->m_actionMap.begin(); itr != _this->m_actionMap.end(); ++itr)
        {
          Action const& action = itr->first;
          int const& key = itr->second;

          if (key >> 8)
          {
            if (GetKeyState(key >> 8) & 0x8000 && GetKeyState(key & 0xFF) & 0x8000)
              newWantedStates[action] = 1.f;
          }
          else if (GetKeyState(key) & 0x8000)
            newWantedStates[action] = 1.f;
        }
      }
      if (!g_mainHandle->GetUIManager()->IsUIEnabled())
      {
        TD::MouseInput* pMouse = TD::RogueClient::Singleton()->m_pClient->m_pMouseInput;
        float dX = (float)pMouse->m_dX / 10.f;
        float dY = (float)pMouse->m_dY / 10.f;

        if (dX > 0)
          newWantedStates[Camera_YawRight] = dX;
        else
          newWantedStates[Camera_YawLeft] = -dX;

        if (dY > 0)
          newWantedStates[Camera_PitchDown] = dY;
        else
          newWantedStates[Camera_PitchUp] = -dY;
      }
      
      if (_this->m_hasController)
      {
        if (_this->m_controllerType == ControllerType::DirectInput)
        {
          DIJOYSTATE2 diState;
          ZeroMemory(&diState, sizeof(DIJOYSTATE2));

          HRESULT result = _this->lpdiGamepad->GetDeviceState(sizeof(DIJOYSTATE2), &diState);
          if (result != S_OK)
          {
            _this->lpdiGamepad->Release();
            _this->lpdiGamepad = nullptr;
            util::log::Warning("DirectInput controller lost");
            _this->m_hasController = false;
          }
          else
            _this->ParseDInputData(diState);
        }
        else
        {
          XINPUT_STATE xiState;
          ZeroMemory(&xiState, sizeof(XINPUT_STATE));

          DWORD dwResult = XInputGetState(_this->m_xinputID, &xiState);
          if (dwResult != ERROR_SUCCESS)
          {
            _this->m_hasController = false;
            util::log::Warning("Xbox controller lost");
          }
          else
            _this->ParseXInputData(xiState);
        }

        for (auto itr = _this->m_gamepadActionMap.begin(); itr != _this->m_gamepadActionMap.end(); ++itr)
        {
          Action const& action = itr->first;
          GamepadKey const& key = itr->second;

          newWantedStates[action] += _this->m_gamepadKeyState[key];
        }
      }
    }

    memcpy(_this->m_wantedActionStates, newWantedStates, sizeof(newWantedStates));

    for (int i = 0; i < ACTION_COUNT; ++i)
    {
      float& currentState = _this->m_smoothedActionStates[i];
      float& wantedState = _this->m_wantedActionStates[i];

      if (currentState != wantedState)
      {
        if (wantedState > 0)
        {
          currentState += dt.count() / 0.2f;
          if (currentState > wantedState)
            currentState = wantedState;
        }
        else
        {
          currentState -= dt.count() / 0.2f;
          if (currentState < 0)
            currentState = 0;
        }
      }
    }
  
    Sleep(1);
  }
}

float InputManager::GetActionState(Action const& action)
{
  return m_smoothedActionStates[action];
}

bool InputManager::IsKeyDown(Action const& action)
{
  return m_wantedActionStates[action] != 0.f;
}

void InputManager::ControllerThread()
{
  Sleep(1000);

  while (!g_shutdown)
  {
    Sleep(1000);

    if (g_mainHandle->GetInputManager()->m_hasController)
      continue;

    g_mainHandle->GetInputManager()->FindDInputController();
    g_mainHandle->GetInputManager()->FindXInputController();
  }
}

void InputManager::ParseDInputData(DIJOYSTATE2& state)
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

      if (NormalizedLX > 0)
        m_gamepadKeyState[GamepadKey::LeftThumb_XPos] = NormalizedLX * NormalizedMagnitude;
      else
        m_gamepadKeyState[GamepadKey::LeftThumb_XNeg] = -NormalizedLX * NormalizedMagnitude;

      if (NormalizedLY < 0)
        m_gamepadKeyState[GamepadKey::LeftThumb_YPos] = -NormalizedLY * NormalizedMagnitude;
      else
        m_gamepadKeyState[GamepadKey::LeftThumb_YNeg] = NormalizedLY * NormalizedMagnitude;
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

      if (NormalizedLX > 0)
        m_gamepadKeyState[GamepadKey::RightThumb_XPos] = NormalizedLX * NormalizedMagnitude;
      else
        m_gamepadKeyState[GamepadKey::RightThumb_XNeg] = -NormalizedLX * NormalizedMagnitude;

      if (NormalizedLY < 0)
        m_gamepadKeyState[GamepadKey::RightThumb_YPos] = -NormalizedLY * NormalizedMagnitude;
      else
        m_gamepadKeyState[GamepadKey::RightThumb_YNeg] = NormalizedLY * NormalizedMagnitude;

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

    double trigger = ((double)delta) / (65535.0 - 6553 / 2);
    if (trigger > 0)
      m_gamepadKeyState[GamepadKey::LeftTrigger] = trigger;
    else
      m_gamepadKeyState[GamepadKey::RightTrigger] = -trigger;
  }

  m_gamepadKeyState[GamepadKey::LeftThumb]      = (state.rgbButtons[10] == 128) ? 1.0f : 0.f;
  m_gamepadKeyState[GamepadKey::RightThumb]     = (state.rgbButtons[11] == 128) ? 1.0f : 0.f;
  m_gamepadKeyState[GamepadKey::LeftShoulder]   = (state.rgbButtons[4] == 128) ? 1.0f : 0.f;
  m_gamepadKeyState[GamepadKey::RightShoulder]  = (state.rgbButtons[5] == 128) ? 1.0f : 0.f;

  m_gamepadKeyState[GamepadKey::DPad_Left]  = (state.rgdwPOV[0] == 27000) ? 1.0f : 0.f;
  m_gamepadKeyState[GamepadKey::DPad_Right] = (state.rgdwPOV[0] == 9000) ? 1.0f : 0.f;
  m_gamepadKeyState[GamepadKey::DPad_Up]    = (state.rgdwPOV[0] == 0) ? 1.0f : 0.f;
  m_gamepadKeyState[GamepadKey::DPad_Down]  = (state.rgdwPOV[0] == 18000) ? 1.0f : 0.f;

  /*
  for (int i = 0; i < 4; ++i)
    input.Buttons[i] = state.rgbButtons[i] == 128;*/
}

void InputManager::ParseXInputData(XINPUT_STATE& state)
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
      double NormalizedLX = LX / magnitude;
      double NormalizedLY = LY / magnitude;

      if (NormalizedLX > 0)
        m_gamepadKeyState[GamepadKey::LeftThumb_XPos] = NormalizedLX * normalizedMagnitude;
      else
        m_gamepadKeyState[GamepadKey::LeftThumb_XNeg] = -NormalizedLX * normalizedMagnitude;

      if (NormalizedLY > 0)
        m_gamepadKeyState[GamepadKey::LeftThumb_YPos] = NormalizedLY * normalizedMagnitude;
      else
        m_gamepadKeyState[GamepadKey::LeftThumb_YNeg] = -NormalizedLY * normalizedMagnitude;
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

    if (magnitude > 0.0) 
    {
      normalizedMagnitude *= normalizedMagnitude;
      // Determine the direction the controller is pushed
      double NormalizedRX = RX / magnitude;
      double NormalizedRY = RY / magnitude;

      if (NormalizedRX > 0)
        m_gamepadKeyState[GamepadKey::RightThumb_XPos] = NormalizedRX * normalizedMagnitude;
      else
        m_gamepadKeyState[GamepadKey::RightThumb_XNeg] = -NormalizedRX * normalizedMagnitude;

      if (NormalizedRY > 0)
        m_gamepadKeyState[GamepadKey::RightThumb_YPos] = NormalizedRY * normalizedMagnitude;
      else
        m_gamepadKeyState[GamepadKey::RightThumb_YNeg] = -NormalizedRY * normalizedMagnitude;
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
    float trigger = ((double)delta) / (255.0 - XINPUT_GAMEPAD_TRIGGER_THRESHOLD / 2);
    if (trigger > 0)
      m_gamepadKeyState[GamepadKey::LeftTrigger] = trigger;
    else
      m_gamepadKeyState[GamepadKey::RightTrigger] = -trigger;
  }

  m_gamepadKeyState[GamepadKey::LeftThumb]      = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) ? 1.0f : 0.f;
  m_gamepadKeyState[GamepadKey::RightThumb]     = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) ? 1.0f : 0.f;
  m_gamepadKeyState[GamepadKey::LeftShoulder]   = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? 1.0f : 0.f;
  m_gamepadKeyState[GamepadKey::RightShoulder]  = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? 1.0f : 0.f;

  m_gamepadKeyState[GamepadKey::DPad_Left]  = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) ? 1.0f : 0.f;
  m_gamepadKeyState[GamepadKey::DPad_Right] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) ? 1.0f : 0.f;
  m_gamepadKeyState[GamepadKey::DPad_Up]    = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) ? 1.0f : 0.f;
  m_gamepadKeyState[GamepadKey::DPad_Down]  = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) ? 1.0f : 0.f;


  /*
  input.Buttons[0] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_X);
  input.Buttons[1] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A);
  input.Buttons[2] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_B);
  input.Buttons[3] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y);*/
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
      util::log::Write("Found an Xbox controller");
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

  HRESULT hr = lpdi->EnumDevices(DI8DEVCLASS_GAMECTRL, (LPDIENUMDEVICESCALLBACKA)DIEnumDevicesCallback, this, DIEDFL_ATTACHEDONLY);
  if (hr != S_OK)
  {
    util::log::Error("Failed to enumerate DirectInput devices. HRESULT 0x%X", hr);
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
  util::log::Write("Found a DirectInput controller");
  return true;
}

BOOL DIEnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
  InputManager* inputManager = static_cast<InputManager*>(pvRef);

  HRESULT hr = inputManager->CreateDevice(lpddi);
  if (FAILED(hr))
  {
    util::log::Error("Failed to create DirectInput device. HRESULT 0x%X", hr);
    return DIENUM_CONTINUE;
  }

  return DIENUM_STOP;
}

HRESULT InputManager::CreateDevice(LPCDIDEVICEINSTANCE lpddi)
{
  if (!lpdi) return 1;
  return lpdi->CreateDevice(lpddi->guidInstance, &lpdiGamepad, NULL);
}


#include "../imgui/imgui.h"
void InputManager::DrawDebug()
{
  ImGui::SetNextWindowSize(ImVec2(200, 600));
  ImGui::Begin("InputManager Debug", 0);
  {
    for (int i = 0; i < 18; ++i)
    {
      ImGui::Text("Key %d - %.3f", i, m_gamepadKeyState[i]);
    }

  }ImGui::End();
}