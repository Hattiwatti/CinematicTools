#include "Renderer.h"
#include "Util.h"
#include "../Main.h"

#include <WICTextureLoader.h>

ID3D11InputLayout* pIOLayout = nullptr;

Renderer::Renderer()
{
  m_initialized = false;
  util::log::Write("Initializing Renderer");

  m_pDevice = fb::DxRenderer::Singleton()->m_pDevice;
  m_pContext = fb::DxRenderer::Singleton()->m_pContext;

  util::log::Write("ID3D11Device 0x%I64X", m_pDevice);
  util::log::Write("ID3D11DeviceContext 0x%I64X", m_pContext);

  m_xtk.fxFactory = std::make_unique<EffectFactory>(m_pDevice);
  if (m_xtk.fxFactory.get() == nullptr) util::log::Error("Failed to create EffectFactory. GetLastError 0x%X", GetLastError());

  m_xtk.primitiveBatch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(m_pContext);
  if (m_xtk.primitiveBatch.get() == nullptr) util::log::Error("Failed to create PrimiteBatch. GetLastError 0x%X", GetLastError());

  m_xtk.spriteBatch = std::make_unique<SpriteBatch>(m_pContext);
  if (m_xtk.spriteBatch.get() == nullptr) util::log::Error("Failed to create SpriteBatch. GetLastError 0x%X", GetLastError());

  m_xtk.states = std::make_unique<CommonStates>(m_pDevice);
  if (m_xtk.states.get() == nullptr) util::log::Error("Failed to create CommonStates. GetLastError 0x%X", GetLastError());

  m_xtk.fx2D = std::make_unique<BasicEffect>(m_pDevice);
  m_xtk.fx3D = std::make_unique<BasicEffect>(m_pDevice);
  m_xtk.fxModel = std::make_unique<BasicEffect>(m_pDevice);
  m_xtk.fxShape = std::make_unique<BasicEffect>(m_pDevice);

  m_pScreen = fb::DxRenderer::Singleton()->m_pScreen;
  util::log::Write("fb::Screen 0x%I64X", m_pScreen);

  m_backBufferWidth = m_pScreen->m_ScreenInfo.m_Width;
  m_backBufferHeight = m_pScreen->m_ScreenInfo.m_Height;

  util::log::Write("Backbuffer size: %dx%d", m_backBufferWidth, m_backBufferHeight);

  m_xtk.viewport = SimpleMath::Viewport(0, 0, m_backBufferWidth, m_backBufferHeight, 0.1, 1.0f);

  m_xtk.fx2D->SetVertexColorEnabled(true);
  m_xtk.fx2D->SetProjection(XMMatrixOrthographicOffCenterRH(0, (float)m_backBufferWidth, (float)m_backBufferHeight, 0, 0, 1));

  m_xtk.fx3D->SetVertexColorEnabled(true);
  m_xtk.fx3D->DisableSpecular();
  m_xtk.fx3D->SetLightingEnabled(false);

  m_xtk.fxModel = std::make_unique<BasicEffect>(m_pDevice);
  m_xtk.fxModel->SetDiffuseColor(XMVectorSet(1, 0, 0, 1));
  m_xtk.fxModel->EnableDefaultLighting();

  m_xtk.fxShape->SetColorAndAlpha(XMVectorSet(1, 0, 0, 0.2));

  void const* pShaderByteCode = nullptr;
  size_t byteCodeLength = 0;
  ID3D11InputLayout* pInputLayout = nullptr;

  m_xtk.fx2D->GetVertexShaderBytecode(&pShaderByteCode, &byteCodeLength);
  HRESULT hr = m_pDevice->CreateInputLayout(VertexPositionColor::InputElements,
    VertexPositionColor::InputElementCount,
    pShaderByteCode, byteCodeLength,
    m_dx.inputLayout.ReleaseAndGetAddressOf());

  m_xtk.fxShape->GetVertexShaderBytecode(&pShaderByteCode, &byteCodeLength);
  m_pDevice->CreateInputLayout(VertexPositionColor::InputElements,
    VertexPositionColor::InputElementCount,
    pShaderByteCode, byteCodeLength,
    &pIOLayout);

  if (hr != S_OK)
  {
    util::log::Error("Failed to create ID3D11InputLayout. HRESULT 0x%X", hr);
    return;
  }

  D3D11_DEPTH_STENCIL_DESC depthDesc;
  depthDesc.DepthEnable = TRUE;
  depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  depthDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
  depthDesc.StencilEnable = TRUE;
  depthDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
  depthDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
  depthDesc.FrontFace.StencilFunc = D3D11_COMPARISON_LESS_EQUAL;
  depthDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
  depthDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depthDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depthDesc.BackFace.StencilFunc = D3D11_COMPARISON_LESS_EQUAL;
  depthDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
  depthDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depthDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

  hr = m_pDevice->CreateDepthStencilState(&depthDesc, m_dx.depthStencilState.ReleaseAndGetAddressOf());
  if (hr != S_OK)
  {
    util::log::Error("Failed to create ID3D11DepthStencilState. HRESULT 0x%X", hr);
    return;
  }

  m_pDevice->CreateDepthStencilState(&depthDesc, m_dx.depthStencilState_ReadOnly.ReleaseAndGetAddressOf());

  m_lastViewMatrix = XMMatrixIdentity();
  m_initialized = true;
  util::log::Ok("Renderer initialized");
}

void Renderer::SetRenderTarget()
{
  m_pContext->OMSetBlendState(m_xtk.states->AlphaBlend(), Colors::White, 0xFFFFFFFF);
  m_pContext->OMSetDepthStencilState(m_xtk.states->DepthNone(), 0);
  
  fb::Dx11RenderTargetView* pFbRtv = fb::DxRenderer::Singleton()->m_pScreen->m_pDefaultRtv;

  if (m_dx.depthStencilView != nullptr && pFbRtv)
    m_pContext->OMSetRenderTargets(pFbRtv->m_targetCount, pFbRtv->m_renderTargetViews, m_dx.depthStencilView);
}

void Renderer::OnResize()
{
  util::log::Write("Resizing");
  m_isResizing = true;
  m_dtResize = g_mainHandle->GetTime();
}

bool Renderer::CheckBufferSize()
{
  if (m_pScreen->m_ScreenInfo.m_Width == m_backBufferWidth &&
    m_pScreen->m_ScreenInfo.m_Height == m_backBufferHeight &&
    !m_isResizing)
    return true;

  if (m_isResizing)
  {
    boost::chrono::high_resolution_clock::duration dt = g_mainHandle->GetTime() - m_dtResize;
    if (dt.count() > 5.0f)
      m_isResizing = false;

    return false;
  }

  if (m_pScreen->m_ScreenInfo.m_Width != m_pScreen->m_ScreenInfo.m_WindowWidth ||
    m_pScreen->m_ScreenInfo.m_Height != m_pScreen->m_ScreenInfo.m_WindowHeight)
    return false;

  m_backBufferWidth = m_pScreen->m_ScreenInfo.m_Width;
  m_backBufferHeight = m_pScreen->m_ScreenInfo.m_Height;
  m_dx.depthStencilView = nullptr;

  m_xtk.fx2D->SetProjection(XMMatrixOrthographicOffCenterRH(0, (float)m_backBufferWidth, (float)m_backBufferHeight, 0, 0, 1));
  return false;
}

void Renderer::UpdateDepthStencilView(fb::Dx11Texture* pTexture)
{
  if (pTexture->m_Desc.m_Width != m_backBufferWidth) return;

  m_dx.depthStencilView = pTexture->m_pDepthStencilView;
}

void Renderer::DrawLine(float x1, float y1, float x2, float y2, const XMFLOAT4& color)
{
  m_pContext->OMSetBlendState(m_xtk.states->Opaque(), nullptr, 0xFFFFFFFF);
  m_pContext->OMSetDepthStencilState(m_xtk.states->DepthNone(), 0);
  m_pContext->RSSetState(m_xtk.states->CullNone());

  m_xtk.fx2D->Apply(m_pContext);
  m_pContext->IASetInputLayout(m_dx.inputLayout.Get());

  m_xtk.primitiveBatch->Begin();

  VertexPositionColor v1, v2;
  v1.position = XMFLOAT3(x1, y1, 0);
  v2.position = XMFLOAT3(x2, y2, 0);
  v1.color = v2.color = color;

  m_xtk.primitiveBatch->DrawLine(v1, v2);

  m_xtk.primitiveBatch->End();
}

void Renderer::DrawCircle(float x, float y, float inRadius, float outRadius, const XMFLOAT4& color)
{
  m_pContext->OMSetBlendState(m_xtk.states->AlphaBlend(), nullptr, 0xFFFFFFFF);
  m_pContext->OMSetDepthStencilState(m_xtk.states->DepthNone(), 0);
  m_pContext->RSSetState(m_xtk.states->CullNone());

  m_xtk.fx2D->Apply(m_pContext);
  m_pContext->IASetInputLayout(m_dx.inputLayout.Get());

  XMVECTOR sPos = XMVectorSet(x, y, 0, 0);
  XMVECTOR pos = XMVectorSet(cos(0), sin(0), 0, 0);

  VertexPositionColor v1, v2, v3, v4;
  v1.color = v2.color = v3.color = v4.color = color;

  XMStoreFloat3(&v1.position, sPos + pos*inRadius);
  XMStoreFloat3(&v4.position, sPos + pos*(inRadius + outRadius));
  float angle = 0.1745f;
  m_xtk.primitiveBatch->Begin();
  for (int i = 0; i < 36; ++i)
  {
    pos.m128_f32[0] = cos(angle); pos.m128_f32[1] = sin(angle);
    angle += 0.1745;
    XMStoreFloat3(&v2.position, sPos + pos*inRadius);
    XMStoreFloat3(&v3.position, sPos + pos*(inRadius + outRadius));
    m_xtk.primitiveBatch->DrawQuad(v1, v2, v3, v4);
    v1 = v2;
    v4 = v3;
  }
  m_xtk.primitiveBatch->End();
}

void Renderer::Draw3DLine(const XMVECTOR& start, const XMVECTOR& end, const XMFLOAT4& startColor, const XMFLOAT4& endColor)
{
  m_xtk.fx3D->Apply(m_pContext);

  m_pContext->IASetInputLayout(m_dx.inputLayout.Get());
  m_pContext->OMSetBlendState(m_xtk.states->AlphaBlend(), Colors::White, 0xFFFFFFFF);
  m_pContext->OMSetDepthStencilState(m_dx.depthStencilState.Get(), 0);
  m_pContext->RSSetState(m_xtk.states->CullNone());

  m_xtk.primitiveBatch->Begin();

  VertexPositionColor v1, v2;
  XMStoreFloat3(&v1.position, start);
  XMStoreFloat3(&v2.position, end);
  v1.color = startColor;
  v2.color = endColor;

  m_xtk.primitiveBatch->DrawLine(v1, v2);

  m_xtk.primitiveBatch->End();
}

void Renderer::Draw3DCircle(const XMVECTOR& position, const float& radius, const XMFLOAT4& color)
{

}

void Renderer::Draw3DPlane(const XMMATRIX& transform, float width, float height, const XMFLOAT4& color)
{
  /*
  m_xtk.fx3D->Apply(m_pContext);

  m_pContext->IASetInputLayout(m_dx.inputLayout.Get());
  m_pContext->OMSetBlendState(m_xtk.states->AlphaBlend(), Colors::White, 0xFFFFFFFF);
  m_pContext->OMSetDepthStencilState(m_dx.depthStencilState_ReadOnly.Get(), 0);
  m_pContext->RSSetState(m_xtk.states->CullNone());

  VertexPositionColor v1, v2, v3, v4;
  XMStoreFloat3(&v1.position, transform.r[3] + width * transform.r[0] + height * transform.r[1]);
  XMStoreFloat3(&v2.position, transform.r[3] - width * transform.r[0] + height * transform.r[1]);
  XMStoreFloat3(&v3.position, transform.r[3] - width * transform.r[0] - height * transform.r[1]);
  XMStoreFloat3(&v4.position, transform.r[3] + width * transform.r[0] - height * transform.r[1]);
  v1.color = v2.color = v3.color = v4.color = color;

  matrixBuffer.world = XMMatrixTranspose(transform);
  matrixBuffer.view = XMMatrixTranspose(fb::GameRenderer::Singleton()->m_pRenderView->m_viewMatrix);
  matrixBuffer.projection = XMMatrixTranspose(fb::GameRenderer::Singleton()->m_pRenderView->m_ProjectionMatrix);

  D3D11_MAPPED_SUBRESOURCE mappedCBuf;
  if (FAILED(m_pContext->Map(pConstantBuffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedCBuf)))
  {
  util::log::Error("Could not map shader constant buffer");
  return;
  }

  memcpy(mappedCBuf.pData, &matrixBuffer, sizeof(matrixBuffer));
  m_pContext->Unmap(pConstantBuffer, 0);

  m_pContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
  m_pContext->VSSetShader(pTestVS, nullptr, 0);
  m_pContext->PSSetShader(pTestPS, nullptr, 0);

  m_xtk.primitiveBatch->Begin();
  m_xtk.primitiveBatch->DrawQuad(v1, v2, v3, v4);
  m_xtk.primitiveBatch->End();*/
}

void Renderer::DrawRectangle(float x1, float y1, float x2, float y2, const XMFLOAT4& color)
{
  m_pContext->OMSetBlendState(m_xtk.states->AlphaBlend(), nullptr, 0xFFFFFFFF);
  m_pContext->OMSetDepthStencilState(m_xtk.states->DepthNone(), 0);
  m_pContext->RSSetState(m_xtk.states->CullNone());

  m_xtk.fx2D->Apply(m_pContext);
  m_pContext->IASetInputLayout(m_dx.inputLayout.Get());

  m_xtk.primitiveBatch->Begin();

  XMFLOAT3 topLeft, topRight, bottomLeft, bottomRight;
  topLeft = XMFLOAT3(x1, y1, 0);
  topRight = XMFLOAT3(x2, y1, 0);
  bottomLeft = XMFLOAT3(x1, y2, 0);
  bottomRight = XMFLOAT3(x2, y2, 0);

  VertexPositionColor v1, v2, v3, v4;

  v1.position = topLeft;
  v2.position = bottomLeft;
  v3.position = topRight;
  v4.position = bottomRight;

  v1.color = v2.color = v3.color = v4.color = color;

  m_xtk.primitiveBatch->DrawQuad(v1, v3, v4, v2);

  m_xtk.primitiveBatch->End();
}

void Renderer::DrawShape(const GeometricPrimitive& shape, const XMMATRIX& transform, const XMFLOAT4& color, bool wireframe)
{
  fb::GameRenderer* pGameRenderer = fb::GameRenderer::Singleton();
  m_xtk.fxShape->SetWorld(transform);
  m_xtk.fxShape->SetColorAndAlpha(XMLoadFloat4(&color));
  shape.Draw(m_xtk.fxShape.get(), pIOLayout, true, wireframe, [=]
  {
    m_pContext->RSSetState(wireframe ? m_xtk.states->Wireframe() : m_xtk.states->CullNone());
    m_pContext->OMSetDepthStencilState(m_dx.depthStencilState.Get(), 0);
  });
}

//void Renderer::ClearDepth()
//{
 // m_pContext->OMSetDepthStencilState(m_xtk.states->DepthNone(), 0);
//}

void Renderer::DrawCMOModel(const Model& model, const XMMATRIX& transform)
{
  fb::GameRenderer* pGameRenderer = fb::GameRenderer::Singleton();
  fb::RenderView* pRenderView = pGameRenderer->m_pRenderView;

  m_xtk.fxModel->SetWorld(transform);

  model.Draw(m_pContext, *m_xtk.states, transform,
    pRenderView->m_viewMatrix, pRenderView->m_ProjectionMatrix, false, [=]
  {
    m_xtk.fxModel->Apply(m_pContext);
    m_pContext->OMSetDepthStencilState(m_dx.depthStencilState.Get(), 0);
    m_pContext->RSSetState(m_xtk.states->CullCounterClockwise());
    m_pContext->OMSetBlendState(m_xtk.states->Opaque(), Colors::White, 0xFFFFFFFF);
  });
}

void Renderer::DrawLineBuffer(const uint16_t* pIndices, size_t indexCount, const VertexPositionColor* pVertices, size_t vertexCount)
{
  m_xtk.fx3D->Apply(m_pContext);
  m_pContext->IASetInputLayout(m_dx.inputLayout.Get());
  m_pContext->OMSetDepthStencilState(m_dx.depthStencilState.Get(), 0);

  m_xtk.primitiveBatch->Begin();
  m_xtk.primitiveBatch->DrawIndexed(D3D11_PRIMITIVE_TOPOLOGY_LINELIST, pIndices, indexCount, pVertices, vertexCount);
  m_xtk.primitiveBatch->End();
}

void Renderer::UpdateViewMatrix()
{
  m_lastViewMatrix = fb::GameRenderer::Singleton()->m_pRenderView->m_viewMatrix;
  m_xtk.fx3D->SetProjection(fb::GameRenderer::Singleton()->m_pRenderView->m_ProjectionMatrix);
  m_xtk.fx3D->SetView(m_lastViewMatrix);
  m_xtk.fxModel->SetView(m_lastViewMatrix);
  m_xtk.fxModel->SetProjection(fb::GameRenderer::Singleton()->m_pRenderView->m_ProjectionMatrix);
  m_xtk.fxShape->SetView(m_lastViewMatrix);
  m_xtk.fxShape->SetProjection(fb::GameRenderer::Singleton()->m_pRenderView->m_ProjectionMatrix);
}

std::unique_ptr<Model> Renderer::CreateCMOModel(void* pData, size_t szData)
{
  return Model::CreateFromCMO(m_pDevice, (const uint8_t*)pData, szData, *m_xtk.fxFactory);
}

ImageRsc Renderer::CreateImageFromResource(int resourceId)
{
  ImageRsc newRsc;

  void* pData = nullptr;
  DWORD dwSize = 0;

  if (util::GetResource(resourceId, pData, dwSize))
  {
    HRESULT hr = DirectX::CreateWICTextureFromMemory(m_pDevice, (const uint8_t*)pData, (size_t)dwSize,
      newRsc.pResource.GetAddressOf(),
      newRsc.pShaderResourceView.GetAddressOf());

    if (SUCCEEDED(hr))
      return newRsc;
    else
      util::log::Error("CreateWICTextureFromMemory failed. Result 0x%X GetLastError 0x%X", hr, GetLastError());
  }
  else
    util::log::Error("Could not get resource");

  // In case creating from resource fails, we try to create an empty texture instead

  D3D11_TEXTURE2D_DESC texDesc;
  texDesc.Width = texDesc.Height = 1;
  texDesc.MipLevels = texDesc.ArraySize = 1;
  texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  texDesc.Usage = D3D11_USAGE_DEFAULT;
  texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  texDesc.CPUAccessFlags = texDesc.MiscFlags = 0;

  ID3D11Texture2D* pTexture = nullptr;
  m_pDevice->CreateTexture2D(&texDesc, NULL, &pTexture);
  if (pTexture)
  {
    newRsc.pResource.Attach((ID3D11Resource*)pTexture);
    m_pDevice->CreateShaderResourceView(pTexture, NULL, newRsc.pShaderResourceView.GetAddressOf());
  }

  return newRsc;
}

void Renderer::DrawImage(ID3D11ShaderResourceView* pResourceView, float x, float y, float w, float h)
{
  m_xtk.spriteBatch->Begin();

  m_xtk.spriteBatch->Draw(pResourceView, XMFLOAT2(x, y), NULL, Colors::White, 0, XMFLOAT2(0, 0), XMFLOAT2(w, h));

  m_xtk.spriteBatch->End();
}

Renderer::~Renderer()
{
  m_initialized = false;
  m_pDevice = nullptr;
  m_pContext = nullptr;
  m_pScreen = 0;
}
