#include "CharacterController.h"
#include "../Main.h"

static const char* g_sPlayer = "Amanda";

CharacterController::CharacterController() : 
  m_IsPlayerInvisible(false),
  m_ToggleVisibility(false),
  m_FreezeCharacters(false)
{
}

CharacterController::~CharacterController()
{

}

CharacterList CharacterController::GetCharacters()
{
  CharacterList result;
  CATHODE::CharacterManager* pCharacterManager = CATHODE::Main::Singleton()->m_CharacterManager;

  result.Count = pCharacterManager->m_NPCCharacterCount + pCharacterManager->m_PlayerCharacterCount;

  // Should always just have one player
  for (unsigned int i = 0; i < pCharacterManager->m_PlayerCharacterCount; ++i)
  {
    result.Names.push_back(g_sPlayer);
    result.Characters.push_back(pCharacterManager->m_PlayerCharacters[i]);
  }

  for (unsigned int i = 0; i < pCharacterManager->m_NPCCharacterCount; ++i)
  {
    CATHODE::Character* pNPC = pCharacterManager->m_NPCCharacters[i];
    result.Names.push_back(pNPC->m_Name);
    result.Characters.push_back(pNPC);
  }

  return result;
}

void CharacterController::Update()
{
  CATHODE::CharacterManager* pChrMgr = CATHODE::Main::Singleton()->m_CharacterManager;
  if (!pChrMgr) return;

  if (m_IsPlayerInvisible || m_ToggleVisibility)
  {
    if (pChrMgr->m_PlayerCharacterCount > 0)
    {
      pChrMgr->m_PlayerCharacters[0]->m_Hidden = m_IsPlayerInvisible;
      pChrMgr->m_PlayerCharacters[0]->m_Hidden2 = m_IsPlayerInvisible;
      m_ToggleVisibility = false;
    }
  }

  if (m_FreezeCharacters)
  {
    for (unsigned int i = 0; i < pChrMgr->m_NPCCharacterCount; ++i)
    {
      CATHODE::Character* pChr = pChrMgr->m_NPCCharacters[i];
      pChr->m_Animate = 0;
      pChr->m_Active = 0;
    }
  }
}

void CharacterController::OnHotkeyUpdate()
{
  InputSystem* pInput = g_mainHandle->GetInputSystem();

  if (pInput->IsActionDown(Action::ToggleInvisibility))
  {
    ToggleInvisibility();

    while (pInput->IsActionDown(Action::ToggleInvisibility))
      Sleep(100);
  }

  if (pInput->IsActionDown(FreezeCharacters))
  {
    m_FreezeCharacters = !m_FreezeCharacters;
    util::log::Write("Freeze characters: %s", m_FreezeCharacters ? "On" : "Off");

    Sleep(100);
    CATHODE::CharacterManager* pChrMgr = CATHODE::Main::Singleton()->m_CharacterManager;
    for (unsigned int i = 0; i < pChrMgr->m_NPCCharacterCount; ++i)
    {
      CATHODE::Character* pChr = pChrMgr->m_NPCCharacters[i];
      pChr->m_Active = !m_FreezeCharacters;
      pChr->m_Animate = !m_FreezeCharacters;
    }

    while (pInput->IsActionDown(FreezeCharacters))
      Sleep(100);
  }

}

void CharacterController::DrawUI()
{
 
}

void CharacterController::ToggleInvisibility()
{
  m_IsPlayerInvisible = !m_IsPlayerInvisible;
  m_ToggleVisibility = true;

  util::log::Write("Invisibility: %s", m_IsPlayerInvisible ? "On" : "Off");
}