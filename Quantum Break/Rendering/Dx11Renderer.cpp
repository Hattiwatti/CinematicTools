#include "Dx11Renderer.h"
#include "../Main.h"
#include "../imgui/imgui_impl_dx11.h"
#include "../Util/Util.h"
#include "../resource.h"
#include "WICTextureLoader.h"

#include "../Northlight/d3d/Device.h"

using namespace DirectX;

Dx11Renderer::Dx11Renderer()
{
  m_pDevice = nullptr;
  m_pContext = nullptr;
  m_pDepthState = nullptr;
  m_pInputLayoutPrimitive = nullptr;
  m_pInputLayoutShape = nullptr;

  m_drawDepth = false;
  m_depthConstants.start = 1.0f;
  m_depthConstants.end = 20.0f;
}

Dx11Renderer::~Dx11Renderer()
{

}

bool Dx11Renderer::Initialize()
{
  util::log::Write("[Dx11Renderer] Initializing renderer");

  m_pDevice = d3d::Device::GetDevice();
  m_pContext = d3d::Device::GetContext();

  util::log::Write("[Dx11Renderer] ID3D11Device 0x%I64X", m_pDevice);
  util::log::Write("[Dx11Renderer] ID3D11DeviceContext 0x%I64X", m_pContext);

  HWND hwnd = d3d::Device::Singleton()->m_hwnd;
  if (!ImGui_ImplDX11_Init(hwnd, m_pDevice, m_pContext))
  {
    util::log::Error("[Dx11Renderer] ImGui_ImplDX11_Init failed, GetLastError 0x%X", GetLastError());
    return false;
  }

  m_pPrimitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(m_pContext);
  m_pSpriteBatch = std::make_unique<DirectX::SpriteBatch>(m_pContext);
  m_pFxFactory = std::make_unique<EffectFactory>(m_pDevice);
  m_pEffect3DPrimitive = std::make_unique<DirectX::BasicEffect>(m_pDevice);
  m_pEffect3DModel = std::make_unique<DirectX::BasicEffect>(m_pDevice);
  m_pEffect3DShape = std::make_unique<DirectX::BasicEffect>(m_pDevice);
  m_pStates = std::make_unique<DirectX::CommonStates>(m_pDevice);

  m_pEffect3DPrimitive->SetVertexColorEnabled(true);
  m_pEffect3DPrimitive->DisableSpecular();
  m_pEffect3DPrimitive->SetLightingEnabled(false);

  m_pEffect3DModel->SetColorAndAlpha(XMVectorSet(1, 0, 0, 1));
  m_pEffect3DModel->EnableDefaultLighting();

  m_pEffect3DShape->DisableSpecular();
  
  void const* pShaderByteCode = nullptr;
  size_t byteCodeLength = 0;

  m_pEffect3DPrimitive->GetVertexShaderBytecode(&pShaderByteCode, &byteCodeLength);
  HRESULT hr = m_pDevice->CreateInputLayout(VertexPositionColor::InputElements,
    VertexPositionColor::InputElementCount,
    pShaderByteCode, byteCodeLength,
    &m_pInputLayoutPrimitive);

  m_pEffect3DShape->GetVertexShaderBytecode(&pShaderByteCode, &byteCodeLength);
  m_pDevice->CreateInputLayout(VertexPositionColor::InputElements,
    VertexPositionColor::InputElementCount,
    pShaderByteCode, byteCodeLength,
    &m_pInputLayoutShape);

  D3D11_DEPTH_STENCIL_DESC depthDesc;
  depthDesc.DepthEnable = TRUE;
  depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  depthDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
  depthDesc.StencilEnable = FALSE;
  depthDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
  depthDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
  depthDesc.FrontFace.StencilFunc = D3D11_COMPARISON_LESS;
  depthDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
  depthDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depthDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depthDesc.BackFace.StencilFunc = D3D11_COMPARISON_LESS;
  depthDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
  depthDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depthDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

  hr = m_pDevice->CreateDepthStencilState(&depthDesc, &m_pDepthState);
  if (FAILED(hr))
  {
    util::log::Error("[Dx11Renderer] CreateDepthStencilState failed, HRESULT 0x%I64X", hr);
    return false;
  }

  m_ViewMatrix = XMMatrixIdentity();
  m_ProjectionMatrix = XMMatrixIdentity();

  util::log::Ok("[Dx11Renderer] Renderer initialized");
}

void Dx11Renderer::Release()
{
  if (m_pDepthState)
  {
    m_pDepthState->Release();
    m_pDepthState = nullptr;
  }

  if (m_pInputLayoutPrimitive)
  {
    m_pInputLayoutPrimitive->Release();
    m_pInputLayoutPrimitive = nullptr;
  }

  if (m_pInputLayoutShape)
  {
    m_pInputLayoutShape->Release();
    m_pInputLayoutShape = nullptr;
  }

  m_pDevice = nullptr;
  m_pContext = nullptr;
}

GeometricWrapper* Dx11Renderer::CreateCube(float size)
{
  std::unique_ptr<GeometricPrimitive> newShape = GeometricPrimitive::CreateCube(m_pContext, size);
  return new Dx11Geometric(newShape);
}

GeometricWrapper* Dx11Renderer::CreateSphere(float radius)
{
  std::unique_ptr<GeometricPrimitive> newShape = GeometricPrimitive::CreateSphere(m_pContext, radius * 2);
  return new Dx11Geometric(newShape);
}

GeometricWrapper* Dx11Renderer::CreateCone(float width, float height)
{
  std::unique_ptr<GeometricPrimitive> newShape = GeometricPrimitive::CreateCone(m_pContext, width, height);
  return new Dx11Geometric(newShape);
}

ImageWrapper* Dx11Renderer::CreateImageFromResource(int id)
{
  void* pData = nullptr;
  DWORD dwSize = 0;

  ID3D11Texture2D* pTexture = nullptr;
  ID3D11ShaderResourceView* pView = nullptr;

  if (util::GetResource(id, pData, dwSize))
  {
    HRESULT hr = DirectX::CreateWICTextureFromMemory(m_pDevice, (const uint8_t*)pData, (size_t)dwSize,
      reinterpret_cast<ID3D11Resource**>(&pTexture),
      &pView);

    if (FAILED(hr))
      util::log::Error("CreateWICTextureFromMemory failed, HRESULT 0x%X", hr);
    else
      return new Dx11ImageRsc(pTexture, pView);
  }
  else
    util::log::Error("Failed to load resource for image, creating an empty texture");

  // In case creating from resource fails, we try to create an empty texture instead

  D3D11_TEXTURE2D_DESC texDesc;
  texDesc.Width = texDesc.Height = 1;
  texDesc.MipLevels = texDesc.ArraySize = 1;
  texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  texDesc.Usage = D3D11_USAGE_DEFAULT;
  texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  texDesc.CPUAccessFlags = texDesc.MiscFlags = 0;

  HRESULT hr = m_pDevice->CreateTexture2D(&texDesc, NULL, &pTexture);
  if (FAILED(hr))
  {
    util::log::Error("CreateTexture2D failed, HRESULT 0x%X", hr);
    return nullptr;
  }

  hr = m_pDevice->CreateShaderResourceView(pTexture, NULL, &pView);
  if (FAILED(hr))
  {
    util::log::Error("CreateShaderResourceView failed, HRESULT 0x%X", hr);
    return nullptr;
  }

  return new Dx11ImageRsc(pTexture, pView);
}

CMOWrapper* Dx11Renderer::CreateModel(void* pData, unsigned int szData)
{
  std::unique_ptr<Model> pModel = Model::CreateFromCMO(m_pDevice, reinterpret_cast<const uint8_t*>(pData), szData, *m_pFxFactory.get());
  return new Dx11CMOModel(pModel);
}

void Dx11Renderer::DrawGeometric(GeometricWrapper* pShape, const DirectX::XMFLOAT4& color, const DirectX::XMMATRIX& transform, bool wireframe)
{
  m_pEffect3DShape->SetColorAndAlpha(XMLoadFloat4(&color));
  m_pEffect3DShape->SetWorld(transform);

  GeometricPrimitive* pPrimitive = reinterpret_cast<GeometricPrimitive*>(pShape->GetShape());
  pPrimitive->Draw(transform, m_ViewMatrix, m_ProjectionMatrix, XMLoadFloat4(&color), nullptr, wireframe, [=]
  {
    m_pEffect3DShape->Apply(m_pContext);
    m_pContext->IASetInputLayout(m_pInputLayoutShape);
    m_pContext->OMSetDepthStencilState(m_pDepthState, 0);
  });
}

void Dx11Renderer::DrawCMOModel(CMOWrapper* pCMOModel, const DirectX::XMMATRIX& transform)
{
  m_pEffect3DModel->SetWorld(transform);

  Model* pModel = reinterpret_cast<Model*>(pCMOModel->GetModel());
  pModel->Draw(m_pContext, *m_pStates, transform,
    m_ViewMatrix, m_ProjectionMatrix, false, [=]
  {
    m_pEffect3DModel->Apply(m_pContext);
    m_pContext->OMSetDepthStencilState(m_pDepthState, 0);
    m_pContext->RSSetState(m_pStates->CullCounterClockwise());
    m_pContext->OMSetBlendState(m_pStates->AlphaBlend(), Colors::White, 0xFFFFFFFF);
  });
}

void Dx11Renderer::DrawBuffer(const uint16_t* pIndices, size_t indexCount, const void* pVertices, size_t vertexCount)
{
  m_pEffect3DPrimitive->Apply(m_pContext);
  m_pContext->IASetInputLayout(m_pInputLayoutPrimitive);
  m_pContext->OMSetDepthStencilState(m_pDepthState, 0);

  m_pPrimitiveBatch->Begin();
  m_pPrimitiveBatch->DrawIndexed(D3D11_PRIMITIVE_TOPOLOGY_LINELIST, pIndices, indexCount, reinterpret_cast<const VertexPositionColor*>(pVertices), vertexCount);
  m_pPrimitiveBatch->End();
}

void Dx11Renderer::DrawPlane(const XMMATRIX& transform, const XMFLOAT2& size, const DirectX::XMFLOAT4& color)
{
  m_pEffect3DPrimitive->Apply(m_pContext);

  m_pContext->IASetInputLayout(m_pInputLayoutPrimitive);
  m_pContext->OMSetBlendState(m_pStates->Opaque(), Colors::White, 0xFFFFFFFF);
  m_pContext->OMSetDepthStencilState(m_pDepthState, 0);

  m_pPrimitiveBatch->Begin();

  VertexPositionColor v1, v2, v3, v4;
  XMStoreFloat3(&v1.position, transform.r[3] + transform.r[0] * 0.5f * size.x + transform.r[1] * 0.5f * size.y);
  XMStoreFloat3(&v2.position, transform.r[3] - transform.r[0] * 0.5f * size.x + transform.r[1] * 0.5f * size.y);
  XMStoreFloat3(&v3.position, transform.r[3] - transform.r[0] * 0.5f * size.x - transform.r[1] * 0.5f * size.y);
  XMStoreFloat3(&v4.position, transform.r[3] + transform.r[0] * 0.5f * size.x - transform.r[1] * 0.5f * size.y);

  v1.color = v2.color = v3.color = v4.color = color;

  m_pPrimitiveBatch->DrawQuad(v1, v2, v3, v4);

  m_pPrimitiveBatch->End();
}

void Dx11Renderer::SetRenderTarget()
{
  m_pContext->OMSetRenderTargets(1, &m_pRenderTarget, NULL);
}

void Dx11Renderer::UpdateMatrices()
{
 
}

void Dx11Renderer::DrawDepthBuffer()
{

}

void Dx11Renderer::ImGui_BeginFrame()
{
  ImGui_ImplDX11_NewFrame();
}

Dx11Geometric::Dx11Geometric(std::unique_ptr<GeometricPrimitive>& pShape)
{
  m_pShape.swap(pShape);
}

Dx11ImageRsc::Dx11ImageRsc(ID3D11Texture2D* pResource, ID3D11ShaderResourceView* pView)
{
  m_pResource = pResource;
  m_pView = pView;
}

void Dx11Renderer::CreateRenderTarget()
{
  ID3D11Texture2D* pBackBuffer = nullptr;
  HRESULT hr = d3d::Device::Singleton()->m_pSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
  if (FAILED(hr))
  {
    util::log::Error("Failed to retrieve backbuffer from swapchain, HRESULT 0x%X", hr);
    return;
  }

  hr = m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRenderTarget);
  if (FAILED(hr))
    util::log::Error("Failed to create RenderTargetView, HRESULT 0x%X", hr);

  pBackBuffer->Release();
}
