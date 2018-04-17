#pragma once
#include <map>
#include <boost/assign.hpp>
#include <Windows.h>

enum Action
{
  UI_Toggle,

  Camera_Toggle,
  Camera_HideHUD,
  Camera_FreezeTime,

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
  Camera_ForwardSecondary,
  Camera_BackwardSecondary,
  Camera_LeftSecondary,
  Camera_RightSecondary,
  Camera_UpSecondary,
  Camera_DownSecondary,

  Track_CreateKey,
  Track_DeleteKey,
  Track_Play,

  Action_Count
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
(UI_Toggle, "UI_Toggle")
(Camera_Toggle, "Camera_Toggle")
(Camera_HideHUD, "Camera_HideHUD")
(Camera_FreezeTime, "Camera_FreezeTime")
(Camera_Forward, "Camera_Forward")
(Camera_Backward, "Camera_Backward")
(Camera_Left, "Camera_Left")
(Camera_Right, "Camera_Right")
(Camera_Up, "Camera_Up")
(Camera_Down, "Camera_Down")
(Camera_YawLeft, "Camera_YawLeft")
(Camera_YawRight, "Camera_YawRight")
(Camera_PitchUp, "Camera_PitchUp")
(Camera_PitchDown, "Camera_PitchDown")
(Camera_RollLeft, "Camera_RollLeft")
(Camera_RollRight, "Camera_RollRight")
(Camera_IncFov, "Camera_IncFov")
(Camera_DecFov, "Camera_DecFov")
(Camera_ForwardSecondary, "Camera_ForwardSecondary")
(Camera_BackwardSecondary, "Camera_BackwardSecondary")
(Camera_LeftSecondary, "Camera_LeftSecondary")
(Camera_RightSecondary, "Camera_RightSecondary")
(Camera_UpSecondary, "Camera_UpSecondary")
(Camera_DownSecondary, "Camera_DownSecondary")
(Track_CreateKey, "Track_CreateKey")
(Track_DeleteKey, "Track_DeleteKey")
(Track_Play, "Track_Play");


static const std::map<Action, std::string> ActionUIStringMap = boost::assign::map_list_of
(UI_Toggle, "Toggle tools UI")
(Camera_Toggle, "Camera toggle")
(Camera_HideHUD, "Hide HUD")
(Camera_FreezeTime, "Freeze time")
(Camera_Forward, "Move forward")
(Camera_Backward, "Move backward")
(Camera_Left, "Move left")
(Camera_Right, "Move right")
(Camera_Up, "Move up")
(Camera_Down, "Move down")
(Camera_YawLeft, "Yaw left")
(Camera_YawRight, "Yaw right")
(Camera_PitchUp, "Pitch up")
(Camera_PitchDown, "Pitch down")
(Camera_RollLeft, "Roll left")
(Camera_RollRight, "Roll right")
(Camera_IncFov, "Increase FoV")
(Camera_DecFov, "Decrease FoV")
(Camera_ForwardSecondary, "Move forward (2)")
(Camera_BackwardSecondary, "Move backward (2)")
(Camera_LeftSecondary, "Move left (2)")
(Camera_RightSecondary, "Move right (2)")
(Camera_UpSecondary, "Move up (2)")
(Camera_DownSecondary, "Move down (2)")
(Track_CreateKey, "Create track keyframe")
(Track_DeleteKey, "Delete last keyframe")
(Track_Play, "Play camera track");


static const std::map<Action, int> DefaultKeyboardMap = boost::assign::map_list_of
(UI_Toggle, VK_F5)
(Camera_Toggle, VK_INSERT)
(Camera_HideHUD, VK_HOME)
(Camera_FreezeTime, VK_DELETE)
(Camera_Forward, 'W')
(Camera_Backward, 'S')
(Camera_Left, 'A')
(Camera_Right, 'D')
(Camera_Up, VK_SPACE)
(Camera_Down, VK_LCONTROL)
(Camera_YawLeft, VK_LEFT)
(Camera_YawRight, VK_RIGHT)
(Camera_PitchUp, VK_UP)
(Camera_PitchDown, VK_DOWN)
(Camera_RollLeft, VK_NUMPAD1)
(Camera_RollRight, VK_NUMPAD3)
(Camera_IncFov, VK_PRIOR)
(Camera_DecFov, VK_NEXT)
(Camera_ForwardSecondary, VK_NUMPAD8)
(Camera_BackwardSecondary, VK_NUMPAD5)
(Camera_LeftSecondary, VK_NUMPAD4)
(Camera_RightSecondary, VK_NUMPAD6)
(Camera_UpSecondary, VK_NUMPAD9)
(Camera_DownSecondary, VK_NUMPAD7)
(Track_CreateKey, 'F')
(Track_DeleteKey, 'G')
(Track_Play, 'P');

static const std::map<Action, GamepadKey> DefaultGamepadMap = boost::assign::map_list_of
(UI_Toggle, GamepadKey::None)
(Camera_Toggle, GamepadKey::None)
(Camera_HideHUD, GamepadKey::None)
(Camera_FreezeTime, GamepadKey::None)
(Camera_Forward, GamepadKey::LeftThumb_YPos)
(Camera_Backward, GamepadKey::LeftThumb_YNeg)
(Camera_Left, GamepadKey::LeftThumb_XNeg)
(Camera_Right, GamepadKey::LeftThumb_XPos)
(Camera_Up, GamepadKey::RightTrigger)
(Camera_Down, GamepadKey::LeftTrigger)
(Camera_YawLeft, GamepadKey::RightThumb_XNeg)
(Camera_YawRight, GamepadKey::RightThumb_XPos)
(Camera_PitchUp, GamepadKey::RightThumb_YPos)
(Camera_PitchDown, GamepadKey::RightThumb_YNeg)
(Camera_RollLeft, GamepadKey::LeftShoulder)
(Camera_RollRight, GamepadKey::RightShoulder)
(Camera_IncFov, GamepadKey::LeftThumb)
(Camera_DecFov, GamepadKey::RightThumb)
(Camera_ForwardSecondary, GamepadKey::None)
(Camera_BackwardSecondary, GamepadKey::None)
(Camera_LeftSecondary, GamepadKey::None)
(Camera_RightSecondary, GamepadKey::None)
(Camera_UpSecondary, GamepadKey::None)
(Camera_DownSecondary, GamepadKey::None)
(Track_CreateKey, GamepadKey::None)
(Track_DeleteKey, GamepadKey::None)
(Track_Play, GamepadKey::None);