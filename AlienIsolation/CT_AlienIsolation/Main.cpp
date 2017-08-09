#include "Main.h"
#include "Hooks.h"
#include "Util/Log.h"

void Main::Init(HINSTANCE dllHandle)
{
  Log::Init();
  m_dllHandle = dllHandle;

  Log::Write("m_dllHandle 0x%X", m_dllHandle);

  m_pCameraManager = std::make_unique<CameraManager>();
  m_pUIManager = std::make_unique<UIManager>();

  Hooks::Init();

  m_exit = false;
  while (!m_exit)
  {
    Update();
    Sleep(1);
  }
}

void Main::Update()
{
  if(GetAsyncKeyState(VK_INSERT) & 0x8000)
  {
    m_pUIManager->Toggle();

    while(GetAsyncKeyState(VK_INSERT) & 0x8000)
      Sleep(100);
  }
}

Main::Main()
{ }

Main::~Main()
{
}

