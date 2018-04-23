#pragma once
#include "RenderImpl.h"

#include <Effects.h>
#include <GeometricPrimitive.h>
#include <Model.h>
#include <PrimitiveBatch.h>
#include <SpriteBatch.h>
#include <CommonStates.h>
#include <VertexTypes.h>

class Dx11CMOModel : public CMOWrapper
{
public:
  Dx11CMOModel() {};
  ~Dx11CMOModel() {};

  Dx11CMOModel(std::unique_ptr<DirectX::Model>& pModel) { m_pModel.swap(pModel); }

  void* GetModel() { return m_pModel.get(); }

private:
  std::unique_ptr<DirectX::Model> m_pModel;
};

class Dx11Geometric : public GeometricWrapper
{
public:
  Dx11Geometric() { };
  ~Dx11Geometric(){ };
  Dx11Geometric(std::unique_ptr<DirectX::GeometricPrimitive>&);

  void* GetShape() { return m_pShape.get(); }

private:
  std::unique_ptr<DirectX::GeometricPrimitive> m_pShape;
};

class Dx11ImageRsc : public ImageWrapper
{
public:
  Dx11ImageRsc() { };
  ~Dx11ImageRsc() { };

  Dx11ImageRsc(ID3D11Texture2D* pResource, ID3D11ShaderResourceView* pView);
  void* GetView() { return reinterpret_cast<void*>(m_pView); }

private:
  ID3D11Texture2D* m_pResource;
  ID3D11ShaderResourceView* m_pView;
};

class Dx11Renderer : public RendererImpl
{
public:
  Dx11Renderer();
  ~Dx11Renderer();

  bool Initialize(ID3D11Device* pDevice, HWND hwnd);
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
  std::unique_ptr<DirectX::CommonStates> m_pStates;

  ID3D11Device* m_pDevice;
  ID3D11DeviceContext* m_pContext;
  ID3D11DepthStencilState* m_pDepthState;
  ID3D11InputLayout* m_pInputLayoutPrimitive;
  ID3D11InputLayout* m_pInputLayoutShape;

  ID3D11Buffer* m_pDepthConstantBuffer;
  ID3D11PixelShader* m_pDepthPixelShader;

  DirectX::XMMATRIX m_ViewMatrix;
  DirectX::XMMATRIX m_ProjectionMatrix;

public:
  Dx11Renderer(Dx11Renderer const&) = delete;
  void operator=(Dx11Renderer const&) = delete;
};