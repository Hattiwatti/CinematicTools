#include "Config.h"
#include "Util/Util.h"
#include "Main.h"
#include "resource.h"

#include <fstream>

Config::Config(std::string const& fileName /* = "./Cinematic Tools/config.ini" */)
{
  m_isDirty = false;
  m_sFileName = fileName;

  util::log::Write("Loading config.ini");
  m_pReader = std::make_unique<INIReader>(fileName);

  int parseResult = m_pReader->ParseError();
  if (parseResult != 0)
  {
    if (parseResult > 0)
    {
      util::log::Warning("Could not parse config.ini");
      util::log::Warning("Overwriting with default file");
    }
    else
      util::log::Write("Config.ini doesn't exist");

    void* pData = nullptr;
    DWORD szData = 0;
    if (util::GetResource(IDR_DEFAULTCONFIG, pData, szData))
    {
      util::log::Write("Creating config.ini");
      std::string defaultConfig((char*)pData);
      std::fstream file;
      file.open(fileName, std::fstream::in | std::fstream::out | std::fstream::trunc);

      file << defaultConfig;
      file.close();
    }
    else
      util::log::Error("Could not load default config from resources");
  }
}

Config::~Config()
{

}

void Config::Update(double dt)
{
  m_dtCheck += dt;
  if (m_dtCheck < 10.f) return;
  m_dtCheck = 0;

  if (m_isDirty)
  {
    m_isDirty = false;
    Save();
  }
}

void Config::Save()
{
  std::fstream file;
  file.open(m_sFileName, std::fstream::in | std::fstream::out | std::fstream::trunc);

  std::vector<std::string> hotkeyConfig = g_mainHandle->GetInputManager()->GetSettings();
  for (auto line : hotkeyConfig)
    file << line << std::endl;

  file << g_mainHandle->GetCameraManager()->GetConfig() << std::endl;
  file.close();
}