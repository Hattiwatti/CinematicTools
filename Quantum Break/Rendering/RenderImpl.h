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
  virtual bool Initialize() { return true; };
  virtual void Release() {};

  virtual bool CheckBufferSize() { return false; }
  virtual void ImGui_BeginFrame() {};

  virtual GeometricWrapper* CreateCube(float size) { return nullptr; }
  virtual GeometricWrapper* CreateSphere(float radius) { return nullptr; }
  virtual GeometricWrapper* CreateCone(float width, float height) { return nullptr; }
  virtual CMOWrapper* CreateModel(void* pData, unsigned int szData) { return nullptr; }
  virtual ImageWrapper* CreateImageFromResource(int id) { return nullptr; }

  virtual void DrawCMOModel(CMOWrapper* pCMOModel, const DirectX::XMMATRIX& transform) {};
  virtual void DrawGeometric(GeometricWrapper* pShape, const DirectX::XMFLOAT4& color, const DirectX::XMMATRIX& transform, bool wireframe = false) {};
  virtual void DrawBuffer(const uint16_t* pIndices, size_t indexCount, const void* pVertices, size_t vertexCount) {};
  virtual void DrawPlane(const DirectX::XMMATRIX& transform, const DirectX::XMFLOAT2& size, const DirectX::XMFLOAT4& color) {};
  
  virtual void DrawDepthBuffer() {};

  virtual void SetRenderTarget() {};
  virtual void UpdateMatrices() {};

  DepthCBuffer& GetDepthConstants() { return m_depthConstants; }
  bool& GetDepthBool() { return m_drawDepth; }

protected:
  bool m_drawDepth;
  DepthCBuffer m_depthConstants;
};