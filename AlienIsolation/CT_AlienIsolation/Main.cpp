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
  if (!m_pRenderer->Initialize(AI::D3D::Singleton()->m_pDevice, g_gameHwnd))
  {
    util::log::Error("Failed to initalize Dx11Renderer");
    return false;
  }

  return true;
}

void Main::Run()
{
  while (!g_shutdown)
  {
    Sleep(1000);
  }
}

void Main::Release()
{
  m_pRenderer->Release();
}