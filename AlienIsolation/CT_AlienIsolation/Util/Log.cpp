#include "Log.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <stdio.h>

using namespace boost::posix_time;

HANDLE Log::hstdin, Log::hstdout;
FILE* Log::pfstdin;
FILE* Log::pfstdout;

void Log::Init()
{
  AllocConsole();
  freopen_s(&pfstdout, "CONOUT$", "w", stdout);
  freopen_s(&pfstdin, "CONIN$", "r", stdin);
  hstdin = GetStdHandle(STD_INPUT_HANDLE);
  hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
}

void Log::PrintTimeStamp()
{
  ptime now = second_clock::local_time();
  std::string sTimeStamp = to_simple_string(now) + "  ";
  SetConsoleTextAttribute(hstdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
  printf(sTimeStamp.c_str());
}

void Log::PrintMessage(WORD color, const char* format, va_list args)
{
  PrintTimeStamp();
  SetConsoleTextAttribute(hstdout, color);
  vfprintf(stdout, format, args);
  fprintf(stdout, "\n");
}


void Log::Write(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  PrintMessage(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY, format, args);
  va_end(args);
}

void Log::Warning(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  PrintMessage(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, format, args);
  va_end(args);
}

void Log::Error(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  PrintMessage(FOREGROUND_RED | FOREGROUND_INTENSITY, format, args);
  va_end(args);
}

void Log::Ok(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  PrintMessage(FOREGROUND_GREEN | FOREGROUND_INTENSITY, format, args);
  va_end(args);
}