#pragma once
#include <d3d11.h>
#include <string>
#include <unordered_map>
#include <wrl.h>

using namespace Microsoft::WRL;

enum ShaderType
{
  VERTEX_SHADER = 1,
  GEOMETRY_SHADER = 2,
  PIXEL_SHADER = 4
};

struct Shader
{
  std::string Name;
  std::wstring FilePath;
  int Types{ 0 };

  ComPtr<ID3D11GeometryShader> GeometryShader;
  ComPtr<ID3D11PixelShader> PixelShader;
  ComPtr<ID3D11VertexShader> VertexShader;
  ComPtr<ID3D11InputLayout> InputLayout;
};

class ShaderStore
{
public:
  ShaderStore();
  ~ShaderStore();

  void AddShaderFromResource(std::string const& name, int vsId, int gsId, int psId, int typeFlags);
  void AddShaderFromFile(std::string const& name, std::wstring const& filePath, int typeFlags);
  void UseShader(std::string const& name);

  void RecompileShaders();

private:
  std::unordered_map<std::string, Shader> m_Shaders;

public:
  ShaderStore(ShaderStore const&) = delete;
  void operator=(ShaderStore const&) = delete;
};