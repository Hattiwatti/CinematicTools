#include "UIManager.h"
#include "Main.h"
#include "AlienIsolation.h"
#include "Util/Log.h"
#include "ImGui/imgui_impl_dx11.h"

#include <CommonStates.h>
#include <Effects.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#pragma comment(lib, "DirectXTK.lib")

std::unique_ptr< PrimitiveBatch< VertexPositionColor > > m_PrimitiveBatch;
std::unique_ptr<CommonStates> m_States;
std::unique_ptr<BasicEffect> m_Effect;
ID3D11InputLayout* m_pInputLayout;

UIManager::UIManager()
{
  m_initialized = false;

  IDXGISwapChain* pSwapChain = AI::Rendering::GetSwapChain();
  ID3D11DeviceContext* pContext = AI::Rendering::GetContext();
  ID3D11Device* pDevice = AI::Rendering::GetDevice();

  printf("0x%X\n", pSwapChain);
  printf("0x%X\n", pContext);
  printf("0x%X\n", pDevice);

  Log::Write("IDXGISwapChain 0x%X", pSwapChain);
  Log::Write("ID3D11DeviceContext 0x%X", pContext);
  Log::Write("ID3D11Device 0x%X", pDevice);

  ID3D11Device* pDevice_Swap = nullptr;
  ID3D11DeviceContext* pContext_Swap = nullptr;
  pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice_Swap);
  pDevice_Swap->GetImmediateContext(&pContext_Swap);

  Log::Write("ID3D11Device through SwapChain 0x%X", pDevice_Swap);
  Log::Write("ID3D11DeviceContext through SwapChain 0x%X", pContext_Swap);

  m_States = std::make_unique<CommonStates>(pDevice);
  m_Effect = std::make_unique<BasicEffect>(pDevice);
  m_Effect->SetVertexColorEnabled(true);
  m_Effect->SetProjection(XMMatrixOrthographicOffCenterRH(0.f, (float)1920, (float)1077, 0.f, 0.f, 1.f));

  Log::Write("HWND 0x%X", AI::Rendering::GetHwnd());
  Log::Write("FindWindow 0x%X", FindWindow(L"Alien: Isolation", NULL));

  void const* shaderByteCode;
  size_t byteCodeLength;

  m_Effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

  if (FAILED(pDevice->CreateInputLayout(VertexPositionColor::InputElements,
    VertexPositionColor::InputElementCount,
    shaderByteCode, byteCodeLength,
    &m_pInputLayout)))
  {
    MessageBox(nullptr, L"m_pDevice->CreateInputLayout failed", L"Error", MB_OK | MB_ICONERROR);
    return;
  }

  m_PrimitiveBatch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(pContext);

  if (!ImGui_ImplDX11_Init((void*)FindWindow(L"Alien: Isolation", NULL), pDevice, pContext))
  {
    Log::Error("Failed to initialize ImGui DX11 handler. GetLastError 0x%X", GetLastError());
    return;
  }

  DWORD thread_id = GetWindowThreadProcessId(FindWindowA("Alien: Isolation", NULL), NULL);
  if (!thread_id)
    thread_id = GetWindowThreadProcessId(FindWindowA(NULL, "Alien: Isolation"), NULL);

  if (thread_id)
  {
    if (!(hGetMessage = SetWindowsHookEx(WH_GETMESSAGE, this->GetMessage_Callback, g_mainHandle->GetDllHandle(), thread_id)))
      Log::Write("Couldn't create WH_GETMESSAGE hook. LastError 0x%X", GetLastError());
  }
  else 
    Log::Warning("Could not find window thread handle");

  m_enabled = false;
  m_hasKeyboardFocus = false;
  m_hasMouseFocus = false;

  ImGuiStyle& Style = ImGui::GetStyle();
  Style.WindowRounding = 0.0f;
  Style.ChildWindowRounding = 0.0f;
  Style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
  Style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);

  m_initialized = true;
}

bool checked = false;
const float color[4] = { 0.f, 0.f, 0.f, 1.f };
float test = 1.0f;


void UIManager::Draw()
{
  if (!m_enabled || !m_initialized) return;

  /*
  ID3D11RenderTargetView* pRtv = nullptr;
  ImGui_ImplDX11_GetContext()->OMGetRenderTargets(1, &pRtv, nullptr);

  ImGui_ImplDX11_GetContext()->ClearRenderTargetView(pRtv, color);

  ImGui_ImplDX11_GetContext()->OMSetBlendState(m_States->AlphaBlend(), nullptr, 0xFFFFFFFF);
  ImGui_ImplDX11_GetContext()->OMSetDepthStencilState(m_States->DepthNone(), 0);
  ImGui_ImplDX11_GetContext()->RSSetState(m_States->CullNone());

  m_Effect->Apply(ImGui_ImplDX11_GetContext());
  ImGui_ImplDX11_GetContext()->IASetInputLayout(m_pInputLayout);

  m_PrimitiveBatch->Begin();

  XMFLOAT3 topLeft(10, 10, 0);
  XMFLOAT3 bottomRight(800,800,0);
  XMFLOAT3 topRight(800, 10, 0.f);
  XMFLOAT3 bottomLeft(10, 800, 0.f);

  VertexPositionColor v1, v2, v3, v4;

  v1.position = topLeft;
  v2.position = bottomLeft;
  v3.position = topRight;
  v4.position = bottomRight;

  v1.color = v2.color = v3.color = v4.color = XMFLOAT4(1, 1, 1, 0.5f);

  m_PrimitiveBatch->DrawQuad(v1, v3, v4, v2);

  m_PrimitiveBatch->End();*/

  ImGui_ImplDX11_NewFrame();
  {
    ImGui::ShowTestWindow();

    ImGui::SetNextWindowSize(ImVec2(300, 500));
    ImGui::Begin("Minimap Generator", nullptr);
    {
      ImGui::InputFloat("Camera Speed", &test, 1, 0, 2);

    } ImGui::End();
  }

  ImGui::Render();

  m_hasKeyboardFocus = ImGui::GetIO().WantCaptureKeyboard;
  m_hasMouseFocus = ImGui::GetIO().WantCaptureMouse;
}

UIManager::~UIManager()
{
  
}

extern LRESULT ImGui_ImplDX11_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall UIManager::GetMessage_Callback(int nCode, WPARAM wParam, LPARAM lParam)
{
  if (AI::Rendering::HasFocus())
  {
    MSG* pMsg = (MSG*)lParam;
    if (!ImGui_ImplDX11_WndProcHandler(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam))
    {
      if (pMsg->message == WM_SIZE)
      {

      }
    }
  }

  return CallNextHookEx(g_mainHandle->GetUIManager()->GetMessageHook(), nCode, wParam, lParam);
}


