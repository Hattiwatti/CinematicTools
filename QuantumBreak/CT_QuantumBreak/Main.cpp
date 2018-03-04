#include "Main.h"
#include "Util/Util.h"

#include "Northlight/ai/AIManager.h"

Main::Main()
{
}

Main::~Main()
{

}

bool Main::Initialize()
{
  util::log::Init();
  util::log::Write("Cinematic Tools for Quantum Break");

  m_pRenderer = std::make_unique<Dx11Renderer>();
  m_pUI = std::make_unique<UI>();

  m_pRenderer->Initialize();
  m_pUI->Initialize(m_pRenderer.get());

  util::hooks::Init();

  return true;
}

bool blindfolded = false;

void Main::Run()
{
  while (!g_shutdown)
  {
    if (GetKeyState(VK_INSERT) & 0x8000)
    {
      blindfolded = !blindfolded;
      util::log::Write("Suspending characters: %s", blindfolded ? "True" : "False");
      ai::Character* pCharacters[256];

      int characterCount = ai::AIManager::GetCharacters(pCharacters);
      if (characterCount > 0)
      {
        util::log::Write("Found %d characters", characterCount);
        for (int i = 0; i < characterCount; ++i)
          pCharacters[i]->SetSuspended(blindfolded);
      }
      else
        util::log::Write("No characters to iterate");

      while (GetKeyState(VK_INSERT) & 0x8000)
        Sleep(10);
    }


    Sleep(1);
  }
}

void Main::Release()
{
  util::hooks::Uninitialize();

  m_pUI->Release();
  m_pRenderer->Release();

  util::log::Write("Main::Release()");
}