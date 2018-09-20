#pragma once
#include <map>
#include <boost/assign.hpp>
#include <Windows.h>

enum Action
{
  ToggleUI,

  ToggleCamera,
  ToggleHUD,
  ToggleFreezeTime,

  Camera_Forward,
  Camera_Backward,
  Camera_Left,
  Camera_Right,
  Camera_Up,
  Camera_Down,

  Camera_ForwardSecondary,
  Camera_BackwardSecondary,
  Camera_LeftSecondary,
  Camera_RightSecondary,
  Camera_UpSecondary,
  Camera_DownSecondary,

  Camera_PitchUp,
  Camera_PitchDown,
  Camera_YawLeft,
  Camera_YawRight,
  Camera_RollLeft,
  Camera_RollRight,

  Camera_IncFocalLength,
  Camera_DecFocalLength,
  Camera_IncAperture,
  Camera_DecAperture,
  Camera_IncFocus,
  Camera_DecFocus,

  Track_CreateNode,
  Track_DeleteNode,
  Track_Play,

  Object_PickUp,
  Object_Rotate,
  Object_Remove,

  ActionCount
};

enum GamepadKey
{
  None,
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
  DPad_Down,
  Button1,
  Button2,
  Button3,
  Button4,
  Button5,
  Button6,
  GamepadKey_Count
};

static const std::map<GamepadKey, std::string> GamepadKeyStrings = boost::assign::map_list_of
(None, "")
(LeftThumb_XPos, "LeftThumb_XPos")
(LeftThumb_XNeg, "LeftThumb_XNeg")
(LeftThumb_YPos, "LeftThumb_YPos")
(LeftThumb_YNeg, "LeftThumb_YNeg")
(RightThumb_XPos, "RightThumb_XPos")
(RightThumb_XNeg, "RightThumb_XNeg")
(RightThumb_YPos, "RightThumb_YPos")
(RightThumb_YNeg, "RightThumb_YNeg")
(LeftTrigger, "LeftTrigger")
(RightTrigger, "RightTrigger")
(LeftThumb, "LeftThumb")
(RightThumb, "RightThumb")
(LeftShoulder, "LeftShoulder")
(RightShoulder, "RightShoulder")
(DPad_Left, "DPad_Left")
(DPad_Right, "DPad_Right")
(DPad_Up, "DPad_Up")
(DPad_Down, "DPad_Down")
(Button1, "X / Square")
(Button2, "A / X")
(Button3, "B / Circle")
(Button4, "Y / Triangle")
(Button5, "Back")
(Button6, "Select");

static const std::map<Action, std::string> ActionStringMap = boost::assign::map_list_of
(ToggleUI, "ToggleUI")
(ToggleCamera, "ToggleCamera")
(ToggleHUD, "ToggleHUD")
(ToggleFreezeTime, "ToggleFreezeTime")
(Camera_Forward, "Camera_Forward")
(Camera_Backward, "Camera_Backward")
(Camera_Left, "Camera_Left")
(Camera_Right, "Camera_Right")
(Camera_Up, "Camera_Up")
(Camera_Down, "Camera_Down")
(Camera_ForwardSecondary, "Camera_ForwardSecondary")
(Camera_BackwardSecondary, "Camera_BackwardSecondary")
(Camera_LeftSecondary, "Camera_LeftSecondary")
(Camera_RightSecondary, "Camera_RightSecondary")
(Camera_UpSecondary, "Camera_UpSecondary")
(Camera_DownSecondary, "Camera_DownSecondary")
(Camera_YawLeft, "Camera_YawLeft")
(Camera_YawRight, "Camera_YawRight")
(Camera_PitchUp, "Camera_PitchUp")
(Camera_PitchDown, "Camera_PitchDown")
(Camera_RollLeft, "Camera_RollLeft")
(Camera_RollRight, "Camera_RollRight")
(Camera_IncFocalLength, "Camera_IncFocalLength")
(Camera_DecFocalLength, "Camera_DecFocalLength")
(Track_CreateNode, "Track_CreateNode")
(Track_DeleteNode, "Track_DeleteNode")
(Track_Play, "Track_Play")
(Object_PickUp, "Object_PickUp")
(Object_Rotate, "Object_Rotate")
(Object_Remove, "Object_Remove")
(Camera_IncAperture, "Camera_IncAperture")
(Camera_DecAperture, "Camera_DecAperture")
(Camera_IncFocus, "Camera_IncFocus")
(Camera_DecFocus, "Camera_DecFocus");


static const std::map<Action, std::string> ActionUIStringMap = boost::assign::map_list_of
(ToggleUI, "Toggle tools UI")
(ToggleCamera, "Camera toggle")
(ToggleHUD, "Hide HUD")
(ToggleFreezeTime, "Freeze time")
(Camera_Forward, "Move forward")
(Camera_Backward, "Move backward")
(Camera_Left, "Move left")
(Camera_Right, "Move right")
(Camera_Up, "Move up")
(Camera_Down, "Move down")
(Camera_ForwardSecondary, "Move forward (2)")
(Camera_BackwardSecondary, "Move backward (2)")
(Camera_LeftSecondary, "Move left (2)")
(Camera_RightSecondary, "Move right (2)")
(Camera_UpSecondary, "Move up (2)")
(Camera_DownSecondary, "Move down (2)")
(Camera_YawLeft, "Yaw left")
(Camera_YawRight, "Yaw right")
(Camera_PitchUp, "Pitch up")
(Camera_PitchDown, "Pitch down")
(Camera_RollLeft, "Roll left")
(Camera_RollRight, "Roll right")
(Camera_IncFocalLength, "Increase focal length")
(Camera_DecFocalLength, "Decrease focal length")
(Track_CreateNode, "Create track node")
(Track_DeleteNode, "Delete track node")
(Track_Play, "Play track")
(Object_PickUp, "Pick up object")
(Object_Rotate, "Rotate object")
(Object_Remove, "Remove object")
(Camera_IncAperture, "Increase lens aperture")
(Camera_DecAperture, "Decrease lens aperture")
(Camera_IncFocus, "Increase focus distance")
(Camera_DecFocus, "Decrease focus distance");


static const std::map<Action, int> DefaultKeyboardMap = boost::assign::map_list_of
(ToggleUI, VK_F5)
(ToggleCamera, VK_INSERT)
(ToggleHUD, VK_HOME)
(ToggleFreezeTime, VK_DELETE)
(Camera_Forward, 'W')
(Camera_Backward, 'S')
(Camera_Left, 'A')
(Camera_Right, 'D')
(Camera_Up, VK_SPACE)
(Camera_Down, VK_LCONTROL)
(Camera_ForwardSecondary, VK_NUMPAD8)
(Camera_BackwardSecondary, VK_NUMPAD5)
(Camera_LeftSecondary, VK_NUMPAD4)
(Camera_RightSecondary, VK_NUMPAD6)
(Camera_UpSecondary, VK_NUMPAD7)
(Camera_DownSecondary, VK_NUMPAD9)
(Camera_YawLeft, VK_LEFT)
(Camera_YawRight, VK_RIGHT)
(Camera_PitchUp, VK_UP)
(Camera_PitchDown, VK_DOWN)
(Camera_RollLeft, VK_NUMPAD1)
(Camera_RollRight, VK_NUMPAD3)
(Camera_IncFocalLength, VK_PRIOR)
(Camera_DecFocalLength, VK_NEXT)
(Track_CreateNode, 'F')
(Track_DeleteNode, 'G')
(Track_Play, 'P')
(Object_PickUp, 'Q')
(Object_Rotate, 'E')
(Object_Remove, 'R')
(Camera_IncAperture, 0)
(Camera_DecAperture, 0)
(Camera_IncFocus, 0)
(Camera_DecFocus, 0);

static const std::map<Action, GamepadKey> DefaultGamepadMap = boost::assign::map_list_of
(ToggleUI, GamepadKey::None)
(ToggleCamera, GamepadKey::None)
(ToggleHUD, GamepadKey::None)
(ToggleFreezeTime, GamepadKey::None)
(Camera_Forward, GamepadKey::LeftThumb_YPos)
(Camera_Backward, GamepadKey::LeftThumb_YNeg)
(Camera_Left, GamepadKey::LeftThumb_XNeg)
(Camera_Right, GamepadKey::LeftThumb_XPos)
(Camera_Up, GamepadKey::RightTrigger)
(Camera_Down, GamepadKey::LeftTrigger)
(Camera_ForwardSecondary, GamepadKey::LeftThumb_YPos)
(Camera_BackwardSecondary, GamepadKey::LeftThumb_YNeg)
(Camera_LeftSecondary, GamepadKey::LeftThumb_XNeg)
(Camera_RightSecondary, GamepadKey::LeftThumb_XPos)
(Camera_UpSecondary, GamepadKey::RightTrigger)
(Camera_DownSecondary, GamepadKey::LeftTrigger)
(Camera_YawLeft, GamepadKey::RightThumb_XNeg)
(Camera_YawRight, GamepadKey::RightThumb_XPos)
(Camera_PitchUp, GamepadKey::RightThumb_YPos)
(Camera_PitchDown, GamepadKey::RightThumb_YNeg)
(Camera_RollLeft, GamepadKey::LeftShoulder)
(Camera_RollRight, GamepadKey::RightShoulder)
(Camera_IncFocalLength, GamepadKey::LeftThumb)
(Camera_DecFocalLength, GamepadKey::RightThumb)
(Track_CreateNode, GamepadKey::None)
(Track_DeleteNode, GamepadKey::None)
(Track_Play, GamepadKey::None)
(Object_PickUp, GamepadKey::None)
(Object_Rotate, GamepadKey::None)
(Object_Remove, GamepadKey::None)
(Camera_IncAperture, GamepadKey::DPad_Left)
(Camera_DecAperture, GamepadKey::DPad_Right)
(Camera_IncFocus, GamepadKey::DPad_Up)
(Camera_DecFocus, GamepadKey::DPad_Down);