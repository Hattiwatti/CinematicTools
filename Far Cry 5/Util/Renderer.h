#pragma once
#include "../Frostbite.h"

#include <boost\chrono.hpp>
#include <CommonStates.h>
#include <Effects.h>
#include <GeometricPrimitive.h>
#include <memory>
#include <Model.h>
#include <PrimitiveBatch.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>
#include <VertexTypes.h>
#include <wrl.h>

#pragma comment(lib, "DirectXTK.lib")

struct ImageRsc
{
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pShaderResourceView;
  Microsoft::WRL::ComPtr<ID3D11Resource> pResource;
};


class Renderer
{
  struct XTKObjects
  {
    std::unique_ptr<EffectFactory> fxFactory;
    std::unique_ptr<BasicEffect> fx2D;
    std::unique_ptr<BasicEffect> fx3D;
    std::shared_ptr<BasicEffect> fxModel;
    std::shared_ptr<BasicEffect> fxShape;

    std::unique_ptr<PrimitiveBatch<VertexPositionColor>> primitiveBatch;
    std::unique_ptr<SpriteBatch> spriteBatch;
    std::unique_ptr<CommonStates> states;

    SimpleMath::Viewport viewport;
  };

  struct DXObjects
  {
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState_ReadOnly;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

    ID3D11DepthStencilView* depthStencilView;
  };

public:
  Renderer();
  ~Renderer();

  void DrawCircle(float x, float y, float inRadius, float outRadius, const XMFLOAT4& color);
  void DrawLine(float x1, float y1, float x2, float y2, const XMFLOAT4& color);
  void DrawRectangle(float x1, float y1, float x2, float y2, const XMFLOAT4& color);

  void Draw3DPlane(const XMMATRIX& transform, float width, float height, const XMFLOAT4& color);
  void Draw3DLine(const XMVECTOR& start, const XMVECTOR& end, const XMFLOAT4& startColor, const XMFLOAT4& endColor);
  inline void Draw3DLine(const XMVECTOR& start, const XMVECTOR& end, const XMFLOAT4& color) { Draw3DLine(start, end, color, color); }
  void Draw3DCircle(const XMVECTOR& position, const float& radius, const XMFLOAT4& color);

  void DrawImage(ID3D11ShaderResourceView* pResourceView, float x, float y, float w, float h);
  void DrawShape(const GeometricPrimitive& shape, const XMMATRIX& transform, const XMFLOAT4& color, bool wireframe = false);
  void DrawCMOModel(const Model& model, const XMMATRIX& transform);

  void DrawLineBuffer(const uint16_t* pIndices, size_t indexCount, const VertexPositionColor* pVertices, size_t vertexCount);

  std::unique_ptr<DirectX::Model> CreateCMOModel(void* pData, size_t szData);
  ImageRsc CreateImageFromResource(int resourceId);

  bool CheckBufferSize();
  void OnResize();
  void SetRenderTarget();
  void UpdateViewMatrix();
  void UpdateDepthStencilView(fb::Dx11Texture*);

private:
  DXObjects m_dx;
  XTKObjects m_xtk;

  ID3D11Device* m_pDevice;
  ID3D11DeviceContext* m_pContext;

  fb::Screen* m_pScreen;
  int m_backBufferWidth;
  int m_backBufferHeight;
  bool m_isResizing;
  XMMATRIX m_lastViewMatrix;

  boost::chrono::high_resolution_clock::time_point m_dtResize;

  bool m_initialized;
  bool recompilestuff;

  fb::Dx11RenderTargetView* pFBRtv;

public:
  Renderer(Renderer const&) = delete;
  void operator=(Renderer const&) = delete;
};
