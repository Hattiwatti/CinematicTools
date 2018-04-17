#include "Main.h"
#include "Dunya.h"
#include "Util/Util.h"
#include <boost/filesystem.hpp>

using namespace boost::chrono;

HINSTANCE FC::FCHandle = NULL;
HWND FC::FCHwnd = NULL;

Main::Main()
{

}

Main::~Main()
{

}

bool Main::Initialize()
{
  boost::filesystem::path dir("Cinematic Tools");
  if (!boost::filesystem::exists(dir))
    boost::filesystem::create_directory(dir);

  util::log::Init();
  util::log::Write("Cinematic Tools for Far Cry 5");

  FC::FCHandle = GetModuleHandleA("FC_m64.dll");
  if (FC::FCHandle == NULL)
  {
    util::log::Error("Could not get module handle for FC_m64.dll");
    return false;
  }

  FC::FCHwnd = FindWindowA("Nomad", NULL);
  if (FC::FCHwnd == NULL)
    FC::FCHwnd = FindWindowA(NULL, "FarCry®5");
  if (FC::FCHwnd == NULL)
  {
    util::log::Error("Could not get window handle");
    return false;
  }

  BYTE Patch[10] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
  util::WriteMemory((DWORD_PTR)((__int64)FC::FCHandle + 0x1E4A474), Patch, 0xA);

  m_pConfig = std::make_unique<Config>();
  m_pCameraManager = std::make_unique<CameraManager>();
  m_pEnvironmentManager = std::make_unique<EnvironmentManager>();
  m_pInputManager = std::make_unique<InputManager>();
  m_pUI = std::make_unique<UI>();

  m_pInputManager->Initialize(m_pConfig->GetReader());
  if (!m_pUI->Initialize(FC::FCHwnd))
  {
    util::log::Error("Failed to initialize UIManager");
    return false;
  }

  util::hooks::Init();
  return true;
}

void Main::Release()
{
  util::log::Write("Main::Release()");
  m_pConfig->Save();

  util::hooks::Uninitialize();

  if (m_pCameraManager.get() != nullptr)
    delete m_pCameraManager.release();
}

void Main::Run()
{

  while (true)
  {
    Update();
    Sleep(1);
  }

  g_shutdown = true;
}

void Main::Update()
{
  boost::chrono::duration<double> dt = high_resolution_clock::now() - m_dtUpdate;
  m_dtUpdate = high_resolution_clock::now();

  m_pCameraManager->Update(dt.count());
  m_pEnvironmentManager->Update();
  m_pUI->Update(dt.count());
}