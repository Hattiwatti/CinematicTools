#include "Main.h"
#include "Hooks.h"
#include "Util/Log.h"

void Main::Init(HINSTANCE dllHandle)
{
  Log::Init();
  m_dllHandle = dllHandle;

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

  // Temporary hotkeys
  if(GetAsyncKeyState(VK_INSERT) & 0x8000)
  {
    m_pCameraManager->ToggleCamera();

    while (GetAsyncKeyState(VK_INSERT) & 0x8000)
      Sleep(100);
  }

  if (GetAsyncKeyState(VK_F1) & 0x8000)
  {
    m_pUIManager->Toggle();

    while (GetAsyncKeyState(VK_F1) & 0x8000)
      Sleep(100);
  }
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

