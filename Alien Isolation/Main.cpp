#include "AlienIsolation.h"
#include "Main.h"
#include "Util/Util.h"

#include <boost/filesystem.hpp>

bool Main::Initialize()
{
  boost::filesystem::path dir("Cinematic Tools");
  if (!boost::filesystem::exists(dir))
    boost::filesystem::create_directory(dir);

  util::log::Init();
  util::log::Write("Cinematic Tools for Alien: Isolation");

  g_gameHwnd = FindWindowA("Alien: Isolation", NULL);
  if (g_gameHwnd == NULL)
  {
    util::log::Error("Could not get handle for game window");
    return false;
  }

  m_pRenderer = std::make_unique<Dx11Renderer>();
  m_pUI = std::make_unique<UI>();

  if (!m_pRenderer->Initialize(AI::D3D::Singleton()->m_pDevice, g_gameHwnd))
  {
    util::log::Error("Failed to initialize Dx11Renderer");
    return false;
  }

  if (!m_pUI->Initialize(m_pRenderer.get(), g_gameHwnd))
  {
    util::log::Error("Failed to initialize UI");
    return false;
  }

  util::hooks::Init();

  return true;
}

void Main::Run()
{
  util::log::Write("Run until shutdown");
  g_shutdown = false;

  while (!g_shutdown)
  {
    Sleep(1000);
  }

  util::log::Write("Shutdown");
}

void Main::Release()
{
  util::log::Write("Release");
  util::hooks::Release();
  m_pRenderer->Release();
}