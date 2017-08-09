#pragma once
#include <Windows.h>

class UIManager
{
public:
  UIManager();
  ~UIManager();

  void Toggle() { m_enabled = !m_enabled; };
  void Draw();

  bool IsEnabled() { return m_enabled; }
  bool IsInitialized() { return m_initialized; }
  bool HasKeyboardFocus() { return m_hasKeyboardFocus; }
  bool HasMouseFocus() { return m_hasMouseFocus; }

  HHOOK GetMessageHook() { return hGetMessage; }

private:
  bool m_enabled;
  bool m_initialized;

  bool m_hasMouseFocus;
  bool m_hasKeyboardFocus;

  HHOOK hGetMessage;
  static LRESULT __stdcall GetMessage_Callback(int nCode, WPARAM wParam, LPARAM lParam);

public:
  UIManager(UIManager const&) = delete;
  void operator=(UIManager const&) = delete;
};