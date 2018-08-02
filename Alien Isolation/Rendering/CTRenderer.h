#pragma once
#include "ShaderStore.h"
#include <CommonStates.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <Effects.h>
#include <GeometricPrimitive.h>
#include <Model.h>
#include <PrimitiveBatch.h>
#include <SpriteBatch.h>
#include <unordered_map>
#include <VertexTypes.h>
#include <wrl.h>

using namespace Microsoft::WRL;

struct ImgRsc
{
  ComPtr<ID3D11Resource> pResource;
  ComPtr<ID3D11ShaderResourceView> pSRV;
};

struct MatrixBuffer
{
  DirectX::XMFLOAT4X4 World;
  DirectX::XMFLOAT4X4 View;
  DirectX::XMFLOAT4X4 Projection;
  DirectX::XMFLOAT4 EyePosition;
};

struct DepthConstants
{
  float Start{ 1.0f };
  float End{ 20.0f };
  bool DrawDepth{ false };
  BYTE Pad009[0x7];
};

class CTRenderer
{
public:
  CTRenderer();
  ~CTRenderer();

  bool Initialize();

  ImgRsc CreateImageFromResource(int id);
  void CreateVertexIndexBuffers(std::vector<unsigned int> const& indices, std::vector<DirectX::VertexPositionColor> const& vertices, ID3D11Buffer** ppVertexBuffer, ID3D11Buffer** ppIndexBuffer);

  std::unique_ptr<DirectX::Model> CreateModelFromResource(int id);

  void DrawGeometric();
  void DrawLines(ID3D11Buffer* pIndexBuffer, ID3D11Buffer* pVertexBuffer, unsigned int indexCount);
  void DrawModel(DirectX::Model* pModel, DirectX::XMMATRIX const& transform, DirectX::XMFLOAT3 const& color);
  void DrawPlane(DirectX::XMMATRIX const& transform, DirectX::XMFLOAT3 const& color, float width, float height);

  void UpdateMatrices();
  void RecompileShaders() { m_Shaders->RecompileShaders(); }

  void DrawDepthBuffer();

  DepthConstants& GetDepthConstants() { return m_DepthConstants; }

private:
  bool CreateRenderTarget();
  ComPtr<ID3D11InputLayout> CreateEffectInputLyout(DirectX::BasicEffect* pEffect);

private:
  ComPtr<ID3D11RenderTargetView> m_RenderTargetView;
  ComPtr<ID3D11DepthStencilState> m_DepthStenciLState;

  std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_PrimitiveBatch;
  std::unique_ptr<DirectX::CommonStates> m_CommonStates;
  std::unique_ptr<DirectX::DGSLEffectFactory> m_EffectFactory;
  std::unique_ptr<DirectX::SpriteBatch> m_SpriteBatch;

  std::unique_ptr<DirectX::BasicEffect> m_ModelEffect;
  std::unique_ptr<DirectX::BasicEffect> m_PrimitiveEffect;
  ComPtr<ID3D11InputLayout> m_ModelEffectIA;
  ComPtr<ID3D11InputLayout> m_PrimitiveEffectIA;

  std::unique_ptr<ShaderStore> m_Shaders;

  MatrixBuffer m_Matrices;
  ComPtr<ID3D11Buffer> m_MatrixBuffer;

  DepthConstants m_DepthConstants;
  ComPtr<ID3D11Buffer> m_DepthCBuffer;

public:
  CTRenderer(CTRenderer const&) = delete;
  void operator=(CTRenderer const&) = delete;
};