#include "OSCReceiver.h"
#include "../Main.h"
#include "../Util/Util.h"
#include <thread>

#pragma comment(lib,"Ws2_32.lib")

const size_t BUFLEN = 512;
const USHORT PORT = 8001;
const u_long MODE = 1; // If != 0, non-blocking is enabled


OSCReceiver::OSCReceiver()
{
  WSAData wsa{ 0 };
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    util::log::Error("Failed to initialize WSA, error code %d", WSAGetLastError());

  m_oscThread = std::thread(&OSCReceiver::ReadData, this);
}

OSCReceiver::~OSCReceiver()
{
  m_oscThread.join();
}

void OSCReceiver::ResetOrigin()
{
  m_OriginPosition = m_TransformData.Position;
  
  XMVECTOR qYaw = util::math::ExtractYaw(XMLoadFloat4(&m_TransformData.Rotation));
  XMStoreFloat4(&m_OriginRotation, qYaw);
}

OSCTransform OSCReceiver::GetTransformData()
{
  OSCTransform data = m_TransformData;

  XMVECTOR absolutePos = XMLoadFloat3(&data.Position);
  XMVECTOR absoluteRot = XMLoadFloat4(&data.Rotation);
  XMVECTOR originPos = XMLoadFloat3(&m_OriginPosition);
  XMVECTOR originRot = XMLoadFloat4(&m_OriginRotation);

  XMVECTOR relativePos = absolutePos - originPos;
  relativePos = XMVector3Rotate(relativePos, XMQuaternionInverse(originRot));

  XMVECTOR relativeRot = XMQuaternionMultiply(absoluteRot, XMQuaternionInverse(originRot));
  relativeRot = XMQuaternionNormalize(relativeRot);

  XMStoreFloat3(&data.Position, relativePos);
  XMStoreFloat4(&data.Rotation, relativeRot);

  return data;
}

void OSCReceiver::BindButton(std::string const& address, std::function<void()> const& func)
{
  m_buttonBindings.emplace(address, func);
}

bool OSCReceiver::CreateSocket()
{
  m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (m_socket == INVALID_SOCKET)
  {
    util::log::Error("Failed to create socket, error code %d", WSAGetLastError());
    return false;
  }

  if (ioctlsocket(m_socket, FIONBIO, (u_long*)&MODE) == SOCKET_ERROR)
    util::log::Warning("Failed to set socket I/O mode to non-blocking");

  sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_port = htons(PORT);
  sin.sin_addr.s_addr = INADDR_ANY;

  if (bind(m_socket, reinterpret_cast<struct sockaddr*>(&sin), sizeof(sockaddr_in)) == SOCKET_ERROR)
  {
    util::log::Error("Failed to bind socket, WSAGetLastError 0x%X\n", WSAGetLastError());
    return false;
  }

  return true;
}

void OSCReceiver::ReadData()
{
  if (!CreateSocket())
    return;
  
  char buffer[BUFLEN];
  while (!g_shutdown)
  {
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(m_socket, &readSet);

    timeval timeout = { 1, 0 };
    if (select(m_socket + 1, &readSet, NULL, NULL, &timeout) > 0)
    {
      sockaddr sa;
      int sa_len = sizeof(sockaddr_in);
      int len = 0;

      while ((len = (int)recvfrom(m_socket, buffer, BUFLEN, 0, &sa, &sa_len)) > 0)
      {
        if (!tosc_isBundle(buffer))
        {
          tosc_message osc;
          tosc_parseMessage(&osc, buffer, len);
          ParseMessage(osc);
        }
      }
    }
  }
}

void OSCReceiver::ParseMessage(tosc_message& msg)
{
  std::string address = tosc_getAddress(&msg);

  if (address == "/position")
  {
    XMVECTOR vPosition;
    for (int i = 0; i < 3; ++i)
      vPosition.m128_f32[i] = tosc_getNextFloat(&msg);

    XMStoreFloat3(&m_TransformData.Position, vPosition);
  }
  else if (address == "/rotation")
  {
    XMVECTOR qRotation;
    for (int i = 0; i < 4; ++i)
      qRotation.m128_f32[i] = tosc_getNextFloat(&msg);

    XMStoreFloat4(&m_TransformData.Rotation, qRotation);
  }
  else
  {
    auto floatBind = m_floatBindings.find(address);
    if (floatBind != m_floatBindings.end())
    {
      float* pFloat = floatBind->second;
      *pFloat = tosc_getNextFloat(&msg);
      return;
    }

    auto boolBind = m_boolBindings.find(address);
    if (boolBind != m_boolBindings.end())
    {
      bool* pBool = boolBind->second;
      *pBool = tosc_getNextInt32(&msg);
      return;
    }

    auto buttonBind = m_buttonBindings.find(address);
    if (buttonBind != m_buttonBindings.end())
    {
      std::function<void()>& func = buttonBind->second;
      func();
    }
    else
      util::log::Warning("OSC received a message with an unknown address %s", address.c_str());
  }
}