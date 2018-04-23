#pragma once
#include "Wrappers.h"
#include <DirectXMath.h>

struct DepthCBuffer
{
  float start;
  float end;
  char Pad008[0x8];
};

class RendererImpl
{
public:
  virtual void Release() = 0;

  virtual void ImGui_BeginFrame() = 0;

  virtual GeometricWrapper* CreateCube(float size) = 0;
  virtual GeometricWrapper* CreateSphere(float radius) = 0;
  virtual GeometricWrapper* CreateCone(float width, float height) = 0;
  virtual CMOWrapper* CreateModel(void* pData, unsigned int szData) = 0;
  virtual ImageWrapper* CreateImageFromResource(int id) = 0;

  virtual void DrawCMOModel(CMOWrapper* pCMOModel, const DirectX::XMMATRIX& transform) = 0;
  virtual void DrawGeometric(GeometricWrapper* pShape, const DirectX::XMFLOAT4& color, const DirectX::XMMATRIX& transform, bool wireframe = false) = 0;
  virtual void DrawBuffer(const uint16_t* pIndices, size_t indexCount, const void* pVertices, size_t vertexCount) = 0;
  virtual void DrawPlane(const DirectX::XMMATRIX& transform, const DirectX::XMFLOAT2& size, const DirectX::XMFLOAT4& color) = 0;
  
  virtual void DrawDepthBuffer() = 0;

  virtual void SetRenderTarget() = 0;
  virtual void UpdateMatrices() = 0;

  DepthCBuffer& GetDepthConstants() { return m_depthConstants; }
  bool& GetDepthBool() { return m_drawDepth; }

protected:
  bool m_drawDepth;
  DepthCBuffer m_depthConstants;
};