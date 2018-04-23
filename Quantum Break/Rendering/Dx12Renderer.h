#pragma once
#include "RenderImpl.h"

#include <IncDx12/GeometricPrimitive.h>
#include <IncDx12/Model.h>
#include <IncDx12/PrimitiveBatch.h>
#include <IncDx12/SpriteBatch.h>
#include <d3d12.h>
/*

class Dx12CMOModel : public CMOWrapper
{
public:
  Dx12CMOModel() {};
  ~Dx12CMOModel() {};

  Dx12CMOModel(std::unique_ptr<DirectX::Model>& pModel) { m_pModel.swap(pModel); }

  void* GetModel() { return m_pModel.get(); }

private:
  std::unique_ptr<DirectX::Model> m_pModel;
};

class Dx12Geometric : public GeometricWrapper
{
public:
  Dx12Geometric() { };
  ~Dx12Geometric() { };
  Dx12Geometric(std::unique_ptr<DirectX::GeometricPrimitive>&);

  void* GetShape() { return m_pShape.get(); }

private:
  std::unique_ptr<DirectX::GeometricPrimitive> m_pShape;
};

class Dx12ImageRsc : public ImageWrapper
{
public:
  Dx12ImageRsc() { };
  ~Dx12ImageRsc() { };

  Dx12ImageRsc(ID3D12Resource* pResource, ID3D12ShaderReflection* pView);
  void* GetView() { return reinterpret_cast<void*>(m_pView); }

private:
  ID3D12Resource * m_pResource;
  * m_pView;
};
*/

class Dx12Renderer : public RendererImpl
{
public:
  Dx12Renderer();
  ~Dx12Renderer();

  bool Initialize();
  void Release();

  void ImGui_BeginFrame();

  GeometricWrapper* CreateCube(float size);
  GeometricWrapper* CreateSphere(float radius);
  GeometricWrapper* CreateCone(float width, float height);
  ImageWrapper* CreateImageFromResource(int id);
  CMOWrapper* CreateModel(void* pData, unsigned int szData);

  void DrawGeometric(GeometricWrapper* pShape, const DirectX::XMFLOAT4& color, const DirectX::XMMATRIX& transform, bool wireframe = false);
  void DrawCMOModel(CMOWrapper* pCMOModel, const DirectX::XMMATRIX& transform);
  void DrawBuffer(const uint16_t* pIndices, size_t indexCount, const void* pVertices, size_t vertexCount);
  void DrawPlane(const DirectX::XMMATRIX& transform, const DirectX::XMFLOAT2& size, const DirectX::XMFLOAT4& color);

  void SetRenderTarget();
  void UpdateMatrices();

  void DrawDepthBuffer();

private:
  std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_pPrimitiveBatch;
  std::unique_ptr<DirectX::SpriteBatch> m_pSpriteBatch;
  std::unique_ptr<DirectX::EffectFactory> m_pFxFactory;
  std::unique_ptr<DirectX::BasicEffect> m_pEffect3DPrimitive;
  std::unique_ptr<DirectX::BasicEffect> m_pEffect3DModel;
  std::unique_ptr<DirectX::BasicEffect> m_pEffect3DShape;

  ID3D12Device* m_pDevice;
  ID3D12CommandList* m_pCommandList;
  //ID3D12DeviceContext* m_pContext;
  //ID3D11DepthStencilState* m_pDepthState;
  //ID3D11InputLayout* m_pInputLayoutPrimitive;
  //ID3D11InputLayout* m_pInputLayoutShape;

  ID3D12Resource* m_pDepthConstantBuffer;
  ID3D12Resource* m_pDepthPixelShader;

  DirectX::XMMATRIX m_ViewMatrix;
  DirectX::XMMATRIX m_ProjectionMatrix;

public:
  Dx12Renderer(Dx12Renderer const&) = delete;
  void operator=(Dx12Renderer const&) = delete;
};