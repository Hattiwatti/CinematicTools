#include "Main.h"
#include "Hooks.h"
#include "Util/Log.h"

void Main::Init(HINSTANCE dllHandle)
{
  Log::Init();
  m_dllHandle = dllHandle;

  Log::Write("m_dllHandle 0x%X", m_dllHandle);

  m_pCameraManager = std::make_unique<CameraManager>();
  m_pInputManager = std::make_unique<InputManager>();
  m_pUIManager = std::make_unique<UIManager>();

  Hooks::Init();

  m_dtUpdate = m_Clock.now();
  m_exit = false;
  while (!m_exit)
  {
    Update();
    Sleep(1);
  }
}

void Main::Update()
{
  duration<double> dt = m_Clock.now() - m_dtUpdate;
  m_dtUpdate = m_Clock.now();

  m_pCameraManager->Update(dt.count());
}

Main::Main()
{ }

Main::~Main()
{
  Hooks::UnInitialize();

  m_pCameraManager.release();
  m_pInputManager.release();
  m_pUIManager.release();
  m_dllHandle = 0;
}

