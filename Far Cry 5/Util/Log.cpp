#include "Util.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <mutex>
#include <stdio.h>

std::mutex g_logMutex;

using namespace boost::posix_time;
using namespace util;
namespace
{
  HANDLE hstdin, hstdout;
  FILE* pfstdin;
  FILE* pfstdout;
  FILE* pfileout;

  void PrintTimeStamp()
  {
    ptime now = second_clock::local_time();
    std::string sTimeStamp = "[" + to_simple_string(now) + "] ";
    SetConsoleTextAttribute(hstdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
    printf(sTimeStamp.c_str());
    fprintf(pfileout, sTimeStamp.c_str());
  }

  void PrintMessage(WORD color, const char* type, const char* format, va_list args)
  {
    std::lock_guard<std::mutex> lock(g_logMutex);
    PrintTimeStamp();
    SetConsoleTextAttribute(hstdout, color);
    std::string finalFormat = std::string(type) + format + "\n";
    vfprintf(stdout, finalFormat.c_str(), args);
    vfprintf(pfileout, finalFormat.c_str(), args);
    fflush(pfileout);
    //fprintf(stdout, "\n");
    //fprintf(pfileout, "\n");
  }
}

void log::Init()
{
  AllocConsole();
  freopen_s(&pfstdout, "CONOUT$", "w", stdout);
  freopen_s(&pfstdin, "CONIN$", "r", stdin);
  hstdin = GetStdHandle(STD_INPUT_HANDLE);
  hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
  pfileout = fopen(".\\Cinematic Tools\\CT.log", "w");
}

void log::Write(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  PrintMessage(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY, "", format, args);
  va_end(args);
}

void log::Warning(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  PrintMessage(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "[WARNING] ", format, args);
  va_end(args);
}

void log::Error(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  PrintMessage(FOREGROUND_RED | FOREGROUND_INTENSITY, "[ERROR] ", format, args);
  va_end(args);
}

void log::Ok(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  PrintMessage(FOREGROUND_GREEN | FOREGROUND_INTENSITY, "[OK] ", format, args);
  va_end(args);
}