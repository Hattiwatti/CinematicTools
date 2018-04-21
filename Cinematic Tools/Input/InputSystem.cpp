#include "InputSystem.h"
#include "../Main.h"
#include <thread>

InputSystem::InputSystem()
{

}

InputSystem::~InputSystem()
{

}

void InputSystem::Initialize()
{
  // Create input-related threads
  std::thread actionThread(&InputSystem::ActionUpdate, this);
  std::thread controllerThread(&InputSystem::ControllerUpdate, this);
  std::thread hotkeyThread(&InputSystem::HotkeyUpdate, this);



  actionThread.detach();
  controllerThread.detach();
  hotkeyThread.detach();
}


void InputSystem::ActionUpdate()
{
  // Action Update thread
  // Processes keyboard + gamepad input and updates
  // action states based on bindings.

  while (!g_shutdown)
  {

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
    if()

    Sleep(1);
  }
}