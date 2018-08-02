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

  Camera_IncFov,
  Camera_DecFov,

  Track_CreateNode,
  Track_DeleteNode,
  Track_Play,

  Object_PickUp,
  Object_Rotate,
  Object_Remove,

  Visuals_IncDofScale,
  Visuals_DecDofScale,
  Visuals_IncDofStrength,
  Visuals_DecDofStrength,
  Visuals_IncFocusDist,
  Visuals_DecFocusDist,

  ToggleInvisibility,
  FreezeCharacters,

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
(Camera_IncFov, "Camera_IncFov")
(Camera_DecFov, "Camera_DecFov")
(Track_CreateNode, "Track_CreateNode")
(Track_DeleteNode, "Track_DeleteNode")
(Track_Play, "Track_Play")
(Object_PickUp, "Object_PickUp")
(Object_Rotate, "Object_Rotate")
(Object_Remove, "Object_Remove")
(Visuals_IncDofScale, "Visuals_IncDofScale")
(Visuals_DecDofScale, "Visuals_DecDofScale")
(Visuals_IncDofStrength, "Visuals_IncDofStrength")
(Visuals_DecDofStrength, "Visuals_DecDofStrength")
(Visuals_IncFocusDist, "Visuals_IncFocusDist")
(Visuals_DecFocusDist, "Visuals_DecFocusDist")
(ToggleInvisibility, "ToggleInvisibility")
(FreezeCharacters, "FreezeCharacters");


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
(Camera_IncFov, "Increase FoV")
(Camera_DecFov, "Decrease FoV")
(Track_CreateNode, "Create track node")
(Track_DeleteNode, "Delete track node")
(Track_Play, "Play track")
(Object_PickUp, "Pick up object")
(Object_Rotate, "Rotate object")
(Object_Remove, "Remove object")
(Visuals_IncDofScale, "Increase DoF scale")
(Visuals_DecDofScale, "Decrease DoF scale")
(Visuals_IncDofStrength, "Increase DoF strength")
(Visuals_DecDofStrength, "Decrease DoF strength")
(Visuals_IncFocusDist, "Increase focus distance")
(Visuals_DecFocusDist, "Decrease focus distance")
(ToggleInvisibility, "Toggle invisibility")
(FreezeCharacters, "Freeze characters");


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
(Camera_ForwardSecondary, 0)
(Camera_BackwardSecondary, 0)
(Camera_LeftSecondary, 0)
(Camera_RightSecondary, 0)
(Camera_UpSecondary, 0)
(Camera_DownSecondary, 0)
(Camera_YawLeft, VK_LEFT)
(Camera_YawRight, VK_RIGHT)
(Camera_PitchUp, VK_UP)
(Camera_PitchDown, VK_DOWN)
(Camera_RollLeft, 0)
(Camera_RollRight, 0)
(Camera_IncFov, VK_ADD)
(Camera_DecFov, VK_SUBTRACT)
(Track_CreateNode, VK_F1)
(Track_DeleteNode, VK_F2)
(Track_Play, VK_F3)
(Object_PickUp, 'Q')
(Object_Rotate, 'E')
(Object_Remove, 'R')
(Visuals_IncDofScale, VK_NUMPAD3)
(Visuals_DecDofScale, VK_NUMPAD2)
(Visuals_IncDofStrength, VK_NUMPAD8)
(Visuals_DecDofStrength, VK_NUMPAD9)
(Visuals_IncFocusDist, VK_NUMPAD6)
(Visuals_DecFocusDist, VK_NUMPAD5)
(ToggleInvisibility, 'I')
(FreezeCharacters, VK_F7);

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
(Camera_IncFov, GamepadKey::LeftThumb)
(Camera_DecFov, GamepadKey::RightThumb)
(Track_CreateNode, GamepadKey::None)
(Track_DeleteNode, GamepadKey::None)
(Track_Play, GamepadKey::None)
(Object_PickUp, GamepadKey::None)
(Object_Rotate, GamepadKey::None)
(Object_Remove, GamepadKey::None)
(Visuals_IncDofScale, GamepadKey::None)
(Visuals_DecDofScale, GamepadKey::None)
(Visuals_IncDofStrength, GamepadKey::None)
(Visuals_DecDofStrength, GamepadKey::None)
(Visuals_IncFocusDist, GamepadKey::None)
(Visuals_DecFocusDist, GamepadKey::None)
(ToggleInvisibility, GamepadKey::None)
(FreezeCharacters, GamepadKey::None);