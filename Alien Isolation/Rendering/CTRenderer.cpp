#include "CTRenderer.h"
#include "../Main.h"
#include "../Util/Util.h"

#include "../resource.h"
#include <WICTextureLoader.h>

CTRenderer::CTRenderer()
{

}

CTRenderer::~CTRenderer()
{

}

bool CTRenderer::Initialize()
{
  util::log::Write("Initializing renderer");

  m_PrimitiveBatch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(g_d3d11Context);
  m_CommonStates = std::make_unique<DirectX::CommonStates>(g_d3d11Device);
  m_EffectFactory = std::make_unique<DirectX::DGSLEffectFactory>(g_d3d11Device);
  m_SpriteBatch = std::make_unique<SpriteBatch>(g_d3d11Context);

  m_PrimitiveEffect = std::make_unique<DirectX::BasicEffect>(g_d3d11Device);
  m_PrimitiveEffect->SetVertexColorEnabled(true);
  m_PrimitiveEffect->DisableSpecular();
  m_PrimitiveEffect->SetLightingEnabled(false);
  m_PrimitiveEffectIA = CreateEffectInputLyout(m_PrimitiveEffect.get());

  m_ModelEffect = std::make_unique<DirectX::BasicEffect>(g_d3d11Device);
  m_ModelEffect->SetColorAndAlpha(XMVectorSet(1, 0, 0, 1));
  m_ModelEffect->EnableDefaultLighting();
  m_ModelEffectIA = CreateEffectInputLyout(m_PrimitiveEffect.get());

  D3D11_DEPTH_STENCIL_DESC depthDesc;
  depthDesc.DepthEnable = FALSE;
  depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
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

  g_d3d11Device->CreateDepthStencilState(&depthDesc, m_DepthStenciLState.GetAddressOf());

  util::log::Write("Creating shaders");
  m_Shaders = std::make_unique<ShaderStore>();
  m_Shaders->AddShaderFromResource("LineShader", IDR_SHADER_LINEVS, IDR_SHADER_LINEGS, IDR_SHADER_LINEPS, VERTEX_SHADER | GEOMETRY_SHADER | PIXEL_SHADER);
  m_Shaders->AddShaderFromResource("DepthShader", 0, 0, IDR_SHADER_ZBUFFER, PIXEL_SHADER);

  XMStoreFloat4x4(&m_Matrices.World, XMMatrixIdentity());
  D3D11_BUFFER_DESC matrixDesc{ 0 }, depthConstantsDesc{ 0 };
  matrixDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  matrixDesc.ByteWidth = sizeof(MatrixBuffer);
  matrixDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  matrixDesc.Usage = D3D11_USAGE_DYNAMIC;

  HRESULT hr = g_d3d11Device->CreateBuffer(&matrixDesc, 0, m_MatrixBuffer.GetAddressOf());
  if (FAILED(hr))
  {
    util::log::Error("Failed to create constant matrix buffer, HRESULT 0x%X", hr);
    return false;
  }

  depthConstantsDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  depthConstantsDesc.ByteWidth = sizeof(DepthConstants);
  depthConstantsDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  depthConstantsDesc.Usage = D3D11_USAGE_DYNAMIC;

  hr = g_d3d11Device->CreateBuffer(&depthConstantsDesc, 0, m_DepthCBuffer.GetAddressOf());
  if (FAILED(hr))
  {
    util::log::Error("Failed to create depth constants buffer, HRESULT 0x%X", hr);
    return false;
  }

  util::log::Ok("Renderer initialized");
  return true;
}

void CTRenderer::UpdateMatrices()
{
  Camera const& camera = g_mainHandle->GetCameraManager()->GetCamera();

  XMVECTOR qRotation = XMLoadFloat4(&camera.AbsoluteRotation);
  XMVECTOR vEyePos = XMLoadFloat3(&camera.AbsolutePosition);

  XMMATRIX rotMatrix = XMMatrixRotationQuaternion(qRotation);
  XMMATRIX viewMatrix = XMMatrixLookToRH(vEyePos, rotMatrix.r[2], XMVectorSet(0, 1, 0, 0));
  XMMATRIX projMatrix = XMMatrixPerspectiveFovRH(camera.Profile.FieldOfView, 1920 / 1080.f, 0.01f, 1000.f);

  m_Matrices.EyePosition = XMFLOAT4(camera.AbsolutePosition.x, camera.AbsolutePosition.y, camera.AbsolutePosition.z, 1);
  XMStoreFloat4x4(&m_Matrices.View, viewMatrix);
  XMStoreFloat4x4(&m_Matrices.Projection, projMatrix);

  m_PrimitiveEffect->SetProjection(projMatrix);
  m_PrimitiveEffect->SetView(viewMatrix);
  m_ModelEffect->SetProjection(projMatrix);
  m_ModelEffect->SetView(viewMatrix);

  D3D11_MAPPED_SUBRESOURCE mappedBuffer;
  g_d3d11Context->Map(m_MatrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);

  MatrixBuffer* pMatrices = reinterpret_cast<MatrixBuffer*>(mappedBuffer.pData);
  XMStoreFloat4x4(&pMatrices->World, XMMatrixIdentity());
  XMStoreFloat4x4(&pMatrices->View, XMMatrixTranspose(viewMatrix));
  XMStoreFloat4x4(&pMatrices->Projection, XMMatrixTranspose(projMatrix));
  pMatrices->EyePosition = m_Matrices.EyePosition;

  g_d3d11Context->Unmap(m_MatrixBuffer.Get(), 0);
}

void CTRenderer::DrawDepthBuffer()
{
//   if (!m_DepthConstants.DrawDepth) return;
// 
//   D3D11_MAPPED_SUBRESOURCE mappedBuffer;
//   g_d3d11Context->Map(m_DepthCBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
//   DepthConstants* pConstants = reinterpret_cast<DepthConstants*>(mappedBuffer.pData);
//   *pConstants = m_DepthConstants;
//   g_d3d11Context->Unmap(m_DepthCBuffer.Get(), 0);
// 
//   m_SpriteBatch->Begin(SpriteSortMode_Deferred, nullptr, nullptr, nullptr, nullptr, [=]
//   {
//     m_Shaders->UseShader("DepthShader");
//     g_d3d11Context->PSSetConstantBuffers(0, 1, m_DepthCBuffer.GetAddressOf());
//   });
//   m_SpriteBatch->Draw(pDepthBuffer->m_ResourceViews[0], XMFLOAT2(0, 0), NULL, Colors::White, 0.0f);
//   m_SpriteBatch->End();
}

ImgRsc CTRenderer::CreateImageFromResource(int id)
{
  ImgRsc newImg;

  DWORD szData; void* pData;
  if (!util::GetResource(id, pData, szData))
  {
    util::log::Error("CreateImageFromResource failed");
    return newImg;
  }

  HRESULT hr = DirectX::CreateWICTextureFromMemory(g_d3d11Device, (const uint8_t*)pData, szData,
    (ID3D11Resource**)newImg.pResource.ReleaseAndGetAddressOf(), newImg.pSRV.ReleaseAndGetAddressOf());

  if (FAILED(hr))
    util::log::Error("CreateWICTextureFromMemory failed");

  return newImg;
}

void CTRenderer::CreateVertexIndexBuffers(std::vector<unsigned int> const& indices, std::vector<DirectX::VertexPositionColor> const& vertices, ID3D11Buffer** ppVertexBuffer, ID3D11Buffer** ppIndexBuffer)
{
  D3D11_BUFFER_DESC indexDesc{ 0 }, vertexDesc{ 0 };
  indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  indexDesc.ByteWidth = indices.size() * sizeof(unsigned int);
  indexDesc.Usage = D3D11_USAGE_DEFAULT;
  indexDesc.StructureByteStride = sizeof(unsigned int);

  vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vertexDesc.ByteWidth = vertices.size() * sizeof(DirectX::VertexPositionColor);
  vertexDesc.Usage = D3D11_USAGE_DEFAULT;
  vertexDesc.StructureByteStride = sizeof(DirectX::VertexPositionColor);

  D3D11_SUBRESOURCE_DATA indexData{ 0 }, vertexData{ 0 };
  indexData.pSysMem = &indices[0];
  vertexData.pSysMem = &vertices[0];

  HRESULT hr = g_d3d11Device->CreateBuffer(&indexDesc, &indexData, ppIndexBuffer);
  if (FAILED(hr))
    util::log::Error("Failed to create index buffer, HRESULT 0x%X", hr);

  hr = g_d3d11Device->CreateBuffer(&vertexDesc, &vertexData, ppVertexBuffer);
  if (FAILED(hr))
    util::log::Error("Failed to create index buffer, HRESULT 0x%X", hr);
}

std::unique_ptr<Model> CTRenderer::CreateModelFromResource(int id)
{
  void* pData = nullptr;
  DWORD szData = 0;

  util::GetResource(id, pData, szData);
  std::unique_ptr<Model> pModel = Model::CreateFromCMO(g_d3d11Device, (const uint8_t*)pData, szData, *m_EffectFactory.get());

  if (pModel == nullptr)
    util::log::Error("Failed to load CMO model from memory");

  return pModel;
}

void CTRenderer::DrawLines(ID3D11Buffer* pIndexBuffer, ID3D11Buffer* pVertexBuffer, unsigned int indexCount)
{
  m_Shaders->UseShader("LineShader");
  g_d3d11Context->GSSetConstantBuffers(0, 1, m_MatrixBuffer.GetAddressOf());
  g_d3d11Context->VSSetConstantBuffers(0, 1, m_MatrixBuffer.GetAddressOf());
  g_d3d11Context->PSSetConstantBuffers(0, 1, m_MatrixBuffer.GetAddressOf());
  //g_d3d11Context->OMSetDepthStencilState(m_DepthStenciLState.Get(), 0);

  const UINT strides = 0x1C;
  const UINT offsets = 0;

  g_d3d11Context->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
  g_d3d11Context->IASetVertexBuffers(0, 1, &pVertexBuffer, &strides, &offsets);
  g_d3d11Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
  g_d3d11Context->DrawIndexed(indexCount, 0, 0);

  g_d3d11Context->GSSetShader(0, 0, 0);
  g_d3d11Context->VSSetShader(0, 0, 0);
  g_d3d11Context->PSSetShader(0, 0, 0);
}

void CTRenderer::DrawModel(DirectX::Model* pModel, DirectX::XMMATRIX const& transform, DirectX::XMFLOAT3 const& color)
{
  m_ModelEffect->SetWorld(transform);

  pModel->Draw(g_d3d11Context, *m_CommonStates, transform, XMLoadFloat4x4(&m_Matrices.View), XMLoadFloat4x4(&m_Matrices.Projection), false, [=]
  {
    m_ModelEffect->Apply(g_d3d11Context);
    g_d3d11Context->OMSetDepthStencilState(m_DepthStenciLState.Get(), 0);
  });
}

void CTRenderer::DrawPlane(DirectX::XMMATRIX const& transform, DirectX::XMFLOAT3 const& color, float width, float height)
{
  m_PrimitiveEffect->Apply(g_d3d11Context);
  g_d3d11Context->IASetInputLayout(m_PrimitiveEffectIA.Get());
  g_d3d11Context->OMSetDepthStencilState(m_DepthStenciLState.Get(), 0);
  m_PrimitiveBatch->Begin();

  VertexPositionColor v1, v2, v3, v4;
  v1.color = v2.color = v3.color = v4.color = XMFLOAT4(color.x, color.y, color.z, 1.0f);
  
  XMVECTOR up = transform.r[1] * 0.5f * height;
  XMVECTOR left = transform.r[0] * 0.5f * width;

  XMVECTOR p1 = transform.r[3] + left - up;
  XMVECTOR p2 = transform.r[3] + left + up;
  XMVECTOR p3 = transform.r[3] - left + up;
  XMVECTOR p4 = transform.r[3] - left - up;

  XMStoreFloat3(&v1.position, p1);
  XMStoreFloat3(&v2.position, p2);
  XMStoreFloat3(&v3.position, p3);
  XMStoreFloat3(&v4.position, p4);

  m_PrimitiveBatch->DrawQuad(v1, v2, v3, v4);
  m_PrimitiveBatch->End();
}

ComPtr<ID3D11InputLayout> CTRenderer::CreateEffectInputLyout(DirectX::BasicEffect* pEffect)
{
  ComPtr<ID3D11InputLayout> pInputLayout;

  void const* pShaderByteCode = nullptr;
  size_t byteCodeLength = 0;
  pEffect->GetVertexShaderBytecode(&pShaderByteCode, &byteCodeLength);

  HRESULT hr = g_d3d11Device->CreateInputLayout(VertexPositionColor::InputElements,
    VertexPositionColor::InputElementCount,
    pShaderByteCode, byteCodeLength, pInputLayout.GetAddressOf());

  if (FAILED(hr))
    util::log::Error("Failed to create InputLayout for BasicEffect, HRESULT 0x%X", hr);

  return pInputLayout;
}