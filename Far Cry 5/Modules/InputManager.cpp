#include "InputManager.h"
#include "../Main.h"
#include "../Util/Util.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

BOOL DIEnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);

InputManager::InputManager()
{
  lpdi = nullptr;
  lpdiGamepad = nullptr;

  m_hasController = false; //FindXInputController() || FindDInputController();

  m_hotkeyWindowFocus = false;
  m_captureKeyboardKey = false;
  m_captureGamepadKey = false;
  m_capturedHotkey = 0;
  m_captureIndex = 0;
  m_sPreviousHotkey = "";
  m_sCapturedHotkey = "";

  ZeroMemory(m_wantedActionStates, sizeof(m_wantedActionStates));
  ZeroMemory(m_smoothActionStates, sizeof(m_smoothActionStates));
  ZeroMemory(m_gamepadKeyStates, sizeof(m_gamepadKeyStates));
  ZeroMemory(m_keyboardMap, sizeof(m_keyboardMap));
  ZeroMemory(m_gamepadMap, sizeof(m_gamepadMap));
}

void InputManager::Initialize(INIReader* pConfigReader)
{
  HRESULT hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8A, (LPVOID*)&lpdi, NULL);
  if (FAILED(hr))
    util::log::Error("Unable to create DirectInput interface. HRESULT 0x%X", hr);

  Rid.usUsagePage = 0x1;
  Rid.usUsage = 0x2;
  Rid.dwFlags = RIDEV_INPUTSINK;
  Rid.hwndTarget = FC::FCHwnd;
  util::log::Write("HWND 0x%X", FC::FCHwnd);
  if (!RegisterRawInputDevices(&Rid, 1, sizeof(Rid)))
  {
    util::log::Warning("RegisterRawInputDevices failed, GetLastError 0x%X", GetLastError());
    util::log::Warning("Mouse input not available");
  }

  ReadConfig(pConfigReader);

  CreateThread(NULL, NULL, HotkeyThread, this, NULL, NULL);
  CreateThread(NULL, NULL, ControllerThread, this, NULL, NULL);
  CreateThread(NULL, NULL, UpdateThread, this, NULL, NULL);
};


void InputManager::ReadConfig(INIReader* pReader)
{
  for (int i = 0; i < Action::Action_Count; ++i)
  {
    Action action = static_cast<Action>(i);
    std::string name = ActionStringMap.at(action);

    int vkey = pReader->GetInteger("KeyboardMap", name, DefaultKeyboardMap.at(action));
    GamepadKey padKey = static_cast<GamepadKey>(pReader->GetInteger("GamepadMap", name, DefaultGamepadMap.at(action)));

    m_keyboardMap[action] = vkey;
    m_gamepadMap[action] = padKey;
  }

  m_sHotkeyMap = new std::string[Action::Action_Count];
  for (int i = 0; i < Action::Action_Count; ++i)
  {
    std::string sHotkey = "";
    int hotkey = m_keyboardMap[static_cast<Action>(i)];
    int modifier = hotkey >> 8;
    hotkey &= 0xFF;

    if (modifier)
      sHotkey += util::VkToString(modifier) + " + ";
    sHotkey += util::VkToString(hotkey);

    m_sHotkeyMap[i] = sHotkey;
  }
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

void InputManager::ParseDInputData(DIJOYSTATE2& state)
{
  // Left Stick
  {
    double lX = (double)state.lX - 32767;
    double lY = (double)state.lY - 32767;

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
        m_gamepadKeyStates[GamepadKey::LeftThumb_XPos] = NormalizedLX * NormalizedMagnitude;
      else
        m_gamepadKeyStates[GamepadKey::LeftThumb_XNeg] = -NormalizedLX * NormalizedMagnitude;

      if (NormalizedLY < 0)
        m_gamepadKeyStates[GamepadKey::LeftThumb_YPos] = -NormalizedLY * NormalizedMagnitude;
      else
        m_gamepadKeyStates[GamepadKey::LeftThumb_YNeg] = NormalizedLY * NormalizedMagnitude;
    }
  }

  // Right Stick
  {
    double lX = (double)state.lZ - 32767;
    double lY = (double)state.lRz - 32767;

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
        m_gamepadKeyStates[GamepadKey::RightThumb_XPos] = NormalizedLX * NormalizedMagnitude;
      else
        m_gamepadKeyStates[GamepadKey::RightThumb_XNeg] = -NormalizedLX * NormalizedMagnitude;

      if (NormalizedLY < 0)
        m_gamepadKeyStates[GamepadKey::RightThumb_YPos] = -NormalizedLY * NormalizedMagnitude;
      else
        m_gamepadKeyStates[GamepadKey::RightThumb_YNeg] = NormalizedLY * NormalizedMagnitude;

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
      m_gamepadKeyStates[GamepadKey::LeftTrigger] = trigger;
    else
      m_gamepadKeyStates[GamepadKey::RightTrigger] = -trigger;
  }

  m_gamepadKeyStates[GamepadKey::LeftThumb] = (state.rgbButtons[10] == 128) ? 1.0f : 0.f;
  m_gamepadKeyStates[GamepadKey::RightThumb] = (state.rgbButtons[11] == 128) ? 1.0f : 0.f;
  m_gamepadKeyStates[GamepadKey::LeftShoulder] = (state.rgbButtons[4] == 128) ? 1.0f : 0.f;
  m_gamepadKeyStates[GamepadKey::RightShoulder] = (state.rgbButtons[5] == 128) ? 1.0f : 0.f;

  m_gamepadKeyStates[GamepadKey::DPad_Left] = (state.rgdwPOV[0] == 27000) ? 1.0f : 0.f;
  m_gamepadKeyStates[GamepadKey::DPad_Right] = (state.rgdwPOV[0] == 9000) ? 1.0f : 0.f;
  m_gamepadKeyStates[GamepadKey::DPad_Up] = (state.rgdwPOV[0] == 0) ? 1.0f : 0.f;
  m_gamepadKeyStates[GamepadKey::DPad_Down] = (state.rgdwPOV[0] == 18000) ? 1.0f : 0.f;

  m_gamepadKeyStates[GamepadKey::Button1] = (state.rgbButtons[0] == 128) ? 1.0f : 0.f;
  m_gamepadKeyStates[GamepadKey::Button2] = (state.rgbButtons[1] == 128) ? 1.0f : 0.f;
  m_gamepadKeyStates[GamepadKey::Button3] = (state.rgbButtons[2] == 128) ? 1.0f : 0.f;
  m_gamepadKeyStates[GamepadKey::Button4] = (state.rgbButtons[3] == 128) ? 1.0f : 0.f;
  m_gamepadKeyStates[GamepadKey::Button5] = (state.rgbButtons[8] == 128) ? 1.0f : 0.f;
  m_gamepadKeyStates[GamepadKey::Button6] = (state.rgbButtons[9] == 128) ? 1.0f : 0.f;
}

void InputManager::ParseXInputData(XINPUT_STATE& state)
{
  {
    double LX = state.Gamepad.sThumbLX;
    double LY = state.Gamepad.sThumbLY;

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
        m_gamepadKeyStates[GamepadKey::LeftThumb_XPos] = NormalizedLX * normalizedMagnitude;
      else
        m_gamepadKeyStates[GamepadKey::LeftThumb_XNeg] = -NormalizedLX * normalizedMagnitude;

      if (NormalizedLY > 0)
        m_gamepadKeyStates[GamepadKey::LeftThumb_YPos] = NormalizedLY * normalizedMagnitude;
      else
        m_gamepadKeyStates[GamepadKey::LeftThumb_YNeg] = -NormalizedLY * normalizedMagnitude;
    }
  }

  {
    // Right thumb
    double RX = state.Gamepad.sThumbRX;
    double RY = state.Gamepad.sThumbRY;

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
        m_gamepadKeyStates[GamepadKey::RightThumb_XPos] = NormalizedRX * normalizedMagnitude;
      else
        m_gamepadKeyStates[GamepadKey::RightThumb_XNeg] = -NormalizedRX * normalizedMagnitude;

      if (NormalizedRY > 0)
        m_gamepadKeyStates[GamepadKey::RightThumb_YPos] = NormalizedRY * normalizedMagnitude;
      else
        m_gamepadKeyStates[GamepadKey::RightThumb_YNeg] = -NormalizedRY * normalizedMagnitude;
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
      m_gamepadKeyStates[GamepadKey::RightTrigger] = trigger;
    else
      m_gamepadKeyStates[GamepadKey::LeftTrigger] = -trigger;
  }

  m_gamepadKeyStates[GamepadKey::LeftThumb] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) ? 1.0f : 0.f;
  m_gamepadKeyStates[GamepadKey::RightThumb] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) ? 1.0f : 0.f;
  m_gamepadKeyStates[GamepadKey::LeftShoulder] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? 1.0f : 0.f;
  m_gamepadKeyStates[GamepadKey::RightShoulder] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? 1.0f : 0.f;

  m_gamepadKeyStates[GamepadKey::DPad_Left] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) ? 1.0f : 0.f;
  m_gamepadKeyStates[GamepadKey::DPad_Right] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) ? 1.0f : 0.f;
  m_gamepadKeyStates[GamepadKey::DPad_Up] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) ? 1.0f : 0.f;
  m_gamepadKeyStates[GamepadKey::DPad_Down] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) ? 1.0f : 0.f;


  m_gamepadKeyStates[GamepadKey::Button1] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_X) ? 1.0f : 0.f;
  m_gamepadKeyStates[GamepadKey::Button2] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) ? 1.0f : 0.f;
  m_gamepadKeyStates[GamepadKey::Button3] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) ? 1.0f : 0.f;
  m_gamepadKeyStates[GamepadKey::Button4] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) ? 1.0f : 0.f;
  m_gamepadKeyStates[GamepadKey::Button5] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) ? 1.0f : 0.f;
  m_gamepadKeyStates[GamepadKey::Button6] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_START) ? 1.0f : 0.f;
}

DWORD WINAPI InputManager::UpdateThread(LPVOID lpArg)
{
  InputManager* pInputMgr = reinterpret_cast<InputManager*>(lpArg);
  boost::chrono::high_resolution_clock::time_point lastUpdate = boost::chrono::high_resolution_clock::now();
  while (!g_shutdown)
  {
    boost::chrono::duration<double> dt = boost::chrono::high_resolution_clock::now() - lastUpdate;
    lastUpdate = boost::chrono::high_resolution_clock::now();

    float newWantedStates[Action::Action_Count];

    ZeroMemory(newWantedStates, sizeof(newWantedStates));
    ZeroMemory(pInputMgr->m_gamepadKeyStates, sizeof(pInputMgr->m_gamepadKeyStates));

    if (FC::IsFocused())
    {
      if (pInputMgr->m_hasController)
      {
        if (pInputMgr->m_controllerType == ControllerType::DirectInput)
        {
          DIJOYSTATE2 diState;
          ZeroMemory(&diState, sizeof(DIJOYSTATE2));

          HRESULT result = pInputMgr->lpdiGamepad->GetDeviceState(sizeof(DIJOYSTATE2), &diState);
          if (result != S_OK)
          {
            pInputMgr->lpdiGamepad->Release();
            pInputMgr->lpdiGamepad = nullptr;
            util::log::Warning("DirectInput controller lost");
            pInputMgr->m_hasController = false;
          }
          else
            pInputMgr->ParseDInputData(diState);
        }
        else
        {
          XINPUT_STATE xiState;
          ZeroMemory(&xiState, sizeof(XINPUT_STATE));

          DWORD dwResult = XInputGetState(pInputMgr->m_xinputID, &xiState);
          if (dwResult != ERROR_SUCCESS)
          {
            pInputMgr->m_hasController = false;
            util::log::Warning("Xbox controller lost");
          }
          else
            pInputMgr->ParseXInputData(xiState);
        }
      }

      if (pInputMgr->m_captureGamepadKey)
      {
        for (int i = 0; i < GamepadKey_Count; ++i)
        {
          if (pInputMgr->m_gamepadKeyStates[i] > 0.5f)
          {
            pInputMgr->m_gamepadMap[pInputMgr->m_captureIndex] = static_cast<GamepadKey>(i);
            pInputMgr->m_captureGamepadKey = false;
            break;
          }
        }
      }

      if (!g_mainHandle->GetUI()->HasKeyboardFocus())
      {
        //if (g_mainHandle->GetCameraManager()->IsKbmDisabled() &&
        //  !g_mainHandle->GetUI()->IsEnabled())
        //{

        //}

        for (int i = 0; i < Action::Action_Count; ++i)
        {
          int key = pInputMgr->m_keyboardMap[i];

          //if (!g_mainHandle->GetCameraManager()->IsKbmDisabled() &&
          //  i >= Camera_Forward && i <= Camera_Down)
          //  key = pInputMgr->m_keyboardMap[i + 14];

          if (key >> 8)
          {
            if (GetKeyState(key >> 8) & 0x8000 && GetKeyState(key & 0xFF) & 0x8000)
              newWantedStates[i] = 1.0f;
          }
          else if (GetKeyState(key) & 0x8000)
            newWantedStates[i] = 1.0f;
        }

        if (pInputMgr->m_hasController && g_mainHandle->GetCameraManager()->IsGamepadDisabled())
        {
          for (int i = 0; i < Action::Action_Count; ++i)
          {
            GamepadKey key = pInputMgr->m_gamepadMap[i];
            newWantedStates[i] += pInputMgr->m_gamepadKeyStates[key];
          }
        }
      }
    }

    memcpy(pInputMgr->m_wantedActionStates, newWantedStates, sizeof(newWantedStates));

    for (int i = 0; i < Action::Action_Count; ++i)
    {
      float& currentState = pInputMgr->m_smoothActionStates[i];
      float& wantedState = pInputMgr->m_wantedActionStates[i];

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

  return 0;
}

DWORD WINAPI InputManager::HotkeyThread(LPVOID lpArg)
{
  InputManager* pInputMgr = reinterpret_cast<InputManager*>(lpArg);

  while (!g_shutdown)
  {
    if (FC::IsFocused())
    {
      if (pInputMgr->IsActionDown(UI_Toggle))
      {
        g_mainHandle->GetUI()->Toggle();
        while (pInputMgr->IsActionDown(UI_Toggle))
          Sleep(1);
      }

      g_mainHandle->GetCameraManager()->HotkeyUpdate();
    }

    Sleep(1);
  }

  util::log::Write("HotkeyThread exit");
  return 0;
}

DWORD WINAPI InputManager::ControllerThread(LPVOID lpArg)
{
  InputManager* pInputMgr = reinterpret_cast<InputManager*>(lpArg);
  Sleep(1000);

  while (!g_shutdown)
  {
    Sleep(1000);

    if (pInputMgr->m_hasController)
      continue;

    if (pInputMgr->FindXInputController())
      continue;

    if (pInputMgr->FindDInputController())
      continue;
  }

  util::log::Write("ControllerThread exit");
  return 0;
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

std::vector<std::string> InputManager::GetSettings()
{
  std::vector<std::string> settings;

  settings.push_back("[KeyboardMap]");
  for (auto itr = ActionStringMap.begin(); itr != ActionStringMap.end(); ++itr)
  {
    Action action = itr->first;
    std::string name = itr->second;

    settings.push_back(name + " = " + std::to_string(m_keyboardMap[action]));
  }

  settings.push_back("\n[GamepadMap]");
  for (auto itr = ActionStringMap.begin(); itr != ActionStringMap.end(); ++itr)
  {
    Action action = itr->first;
    std::string name = itr->second;

    settings.push_back(name + " = " + std::to_string(m_gamepadMap[action]));
  }

  return settings;
}

void InputManager::DrawConfigUI()
{
  ImGuiIO& io = ImGui::GetIO();
  ImGui::GetCurrentContext()->InputTextState.CursorAnim = 1.0f;

  ImGui::Columns(2, "configColumns", false);
  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 10);

  ImGui::Dummy(ImVec2(0, 2));

  ImGui::PushFont(io.Fonts->Fonts[4]);
  ImGui::PushItemWidth(150);

  ImGui::Text("Hotkeys"); ImGui::SameLine(140.f, -1.f);
  ImGui::Text("Keyboard"); ImGui::SameLine(298.f, -1.f);
  ImGui::Text("Gamepad");
  int i = 0;

  for (int i = 0; i < Action_Count; ++i)
  {
    Action action = static_cast<Action>(i);
    std::string name = ActionUIStringMap.at(action);
    GamepadKey padKey = m_gamepadMap[i];

    std::string kbString = (m_captureKeyboardKey && m_captureIndex == i) ? "Press a key" : m_sHotkeyMap[i];
    std::string padString = (m_captureGamepadKey && m_captureIndex == i) ? "Press a key" : GamepadKeyStrings.at(padKey);

    ImGui::Text("%s", name.c_str()); ImGui::SameLine(140.f, -1.f);
    name = "##Input" + name;
    ImGui::InputText(name.c_str(), (char*)kbString.c_str(), kbString.size(), ImGuiInputTextFlags_ReadOnly); ImGui::SameLine();
    if (ImGui::IsItemClicked())
      StartKeyboardCapture(i);

    name += "Gamepad";
    ImGui::InputText(name.c_str(), (char*)padString.c_str(), padString.size(), ImGuiInputTextFlags_ReadOnly);
    if (ImGui::IsItemClicked())
      StartGamepadCapture(i);
  }

  ImGui::Dummy(ImVec2(0, 10));
  ImGui::PopFont();
}

bool InputManager::KeyDown(LPARAM lparam, WPARAM wparam)
{
  if (!m_captureKeyboardKey) return false;
  if (wparam == VK_ESCAPE) return false;

  if (wparam == VK_LCONTROL || wparam == VK_RCONTROL || wparam == VK_CONTROL ||
    wparam == VK_LSHIFT || wparam == VK_RSHIFT || wparam == VK_SHIFT ||
    wparam == VK_MENU)
  {
    m_capturedHotkey = wparam << 8;
    m_sCapturedHotkey = util::KeyLparamToString(lparam) + " + ";
    m_sHotkeyMap[m_captureIndex] = m_sCapturedHotkey;
  }
  else
  {
    m_capturedHotkey |= wparam;
    m_sCapturedHotkey += util::KeyLparamToString(lparam);

    m_sHotkeyMap[m_captureIndex] = m_sCapturedHotkey;
    m_keyboardMap[static_cast<Action>(m_captureIndex)] = m_capturedHotkey;
    m_captureKeyboardKey = false;
  }

  return true;
}

void InputManager::StartKeyboardCapture(int index)
{
  if (m_captureKeyboardKey)
    m_sHotkeyMap[m_captureIndex] = m_sPreviousHotkey;

  if (m_captureGamepadKey)
  {
    m_gamepadMap[m_captureIndex] = m_previousGamepadKey;
    m_captureGamepadKey = false;
  }

  m_captureKeyboardKey = true;
  m_sPreviousHotkey = m_sHotkeyMap[index];
  m_sCapturedHotkey = "";
  m_capturedHotkey = 0;
  m_sHotkeyMap[index] = std::string("Press a key");
  m_captureIndex = index;
}

void InputManager::StartGamepadCapture(int index)
{
  if (m_captureKeyboardKey)
  {
    m_sHotkeyMap[m_captureIndex] = m_sPreviousHotkey;
    m_captureKeyboardKey = false;
  }

  if (m_captureGamepadKey)
    m_gamepadMap[m_captureIndex] = m_previousGamepadKey;

  m_previousGamepadKey = m_gamepadMap[index];
  m_captureGamepadKey = true;
  m_captureIndex = index;
}

InputManager::~InputManager()
{
  if (lpdiGamepad != nullptr)
    lpdiGamepad->Release();
  if (lpdi != nullptr)
    lpdi->Release();
}

float InputManager::GetActionState(Action action)
{
  return m_smoothActionStates[action];
}

bool InputManager::IsActionDown(Action action)
{
  return m_wantedActionStates[action] != 0;
}

void InputManager::HandleInputMessage(LPARAM lParam)
{
  RECT windowRect;
  if (!GetClientRect(FC::FCHwnd, &windowRect))
    return;

  signed short xPosRelative = (lParam & 0xFFFF);
  signed short yPosRelative = (lParam >> 16);

  int deltaX = xPosRelative - (windowRect.right - windowRect.left) / 2;
  int deltaY = yPosRelative - (windowRect.bottom - windowRect.top) / 2;
  m_mouseState = std::tuple<int, int>(deltaX, deltaY);
}