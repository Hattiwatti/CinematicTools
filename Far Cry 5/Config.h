#pragma once
#include <memory>
#include "inih/cpp/INIReader.h"

class Config
{
public:
  Config(std::string const& fileName = "./Cinematic Tools/config.ini");
  ~Config();

  void MarkDirty() { m_isDirty = true; }
  void Save();
  void Update(double);

  INIReader* GetReader() { return m_pReader.get(); }

private:
  std::string m_sFileName;
  std::unique_ptr<INIReader> m_pReader;

  float m_dtCheck;
  bool m_isDirty;

public:
  Config(Config const&);
  void operator=(Config const&) = delete;
};