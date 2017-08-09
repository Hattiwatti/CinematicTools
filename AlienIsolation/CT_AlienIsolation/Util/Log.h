#pragma once
#include <Windows.h>
#include <stdio.h>

class Log
{
public:
  static void Init();

  static void Write(const char* format, ...);
  static void Warning(const char* format, ...);
  static void Error(const char* format, ...);
  static void Ok(const char* format, ...);

private:
  static HANDLE hstdin;
  static HANDLE hstdout;
  static FILE* pfstdout;
  static FILE* pfstdin;

  static void PrintTimeStamp();
  static void PrintMessage(WORD color, const char* format, va_list args);
};