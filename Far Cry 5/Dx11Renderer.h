#pragma once
#include <d3d11.h>

class Dx11Renderer
{
public:
  Dx11Renderer();
  ~Dx11Renderer();

  bool Initialize(ID3D11Device* pDevice);
  void SetRenderTarget();

private:

private:

public:
  Dx11Renderer(Dx11Renderer const&) = delete;
  void operator=(Dx11Renderer const&) = delete;
};