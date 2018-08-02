#pragma once
#include "../tinyosc/tinyosc.h"
#include <DirectXMath.h>
#include <unordered_map>
#include <thread>

struct OSCTransform
{
  DirectX::XMFLOAT3 Position{ 0,0,0 };
  DirectX::XMFLOAT4 Rotation{ 0,0,0,1 };
};

struct OSCUIData
{

};

struct OSCButton
{
  std::function<void()> Func;
  bool WentDown;
};

class OSCReceiver
{
public:
  OSCReceiver();
  ~OSCReceiver();

  void ResetOrigin();

  OSCTransform GetTransformData();
  OSCUIData GetUIData() { return m_UIData; }
 
  void BindButton(std::string const&, std::function<void()> const&);

private:

  bool CreateSocket();
  void ReadData();

  void ParseMessage(tosc_message&);

private:
  int m_socket;

  OSCTransform m_TransformData;
  OSCUIData m_UIData;

  DirectX::XMFLOAT3 m_OriginPosition;
  DirectX::XMFLOAT4 m_OriginRotation;

  std::thread m_oscThread;

  std::unordered_map<std::string, float*> m_floatBindings;
  std::unordered_map<std::string, bool*> m_boolBindings;

  std::unordered_map<std::string, std::function<void()>> m_buttonBindings;

public:
  OSCReceiver(OSCReceiver const&) = delete;
  void operator=(OSCReceiver const&) = delete;
};

