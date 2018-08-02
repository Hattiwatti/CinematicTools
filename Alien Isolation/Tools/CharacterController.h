#pragma once
#include "../AlienIsolation.h"

struct CharacterList
{
  unsigned int Count{ 0 };
  std::vector<const char*> Names;
  std::vector<CATHODE::Character*> Characters;
};

class CharacterController
{
public:
  CharacterController();
  ~CharacterController();

  void Update();
  void OnHotkeyUpdate();

  void ShowUI() { m_ShowUI = true; }
  void DrawUI();

  CharacterList GetCharacters();

  bool IsPlayerInvisible() { return m_IsPlayerInvisible; }

private:
  void ToggleInvisibility();

private:
  bool m_ShowUI;

  bool m_IsPlayerInvisible;
  // Needs an extra bool because otherwise it stays invisible
  // forever.
  bool m_ToggleVisibility;
  bool m_FreezeCharacters;

public:
  CharacterController(CharacterController const&) = delete;
  void operator=(CharacterController const&) = delete;
};