#include "ShaderStore.h"
#include "../Globals.h"
#include "../Util/Util.h"

#include <fstream>

static void PrintErrorBlob(ID3D10Blob* pError)
{
  if (pError)
  {
    char* pCompileErrors = static_cast<char*>(pError->GetBufferPointer());
    util::log::Write("%s", pCompileErrors);
  }
}

static void DumpShaderCode(std::string const& name, ID3D10Blob* pCode)
{
  std::ofstream fout;
  fout.open(name.c_str(), std::ios::binary | std::ios::out);

  fout.write((char*)pCode->GetBufferPointer(), pCode->GetBufferSize());
  fout.close();
}

ShaderStore::ShaderStore()
{

}

ShaderStore::~ShaderStore()
{

}

void ShaderStore::AddShaderFromFile(std::string const& name, std::wstring const& filePath, int typeFlags)
{
//   HRESULT hr;
//   ID3D10Blob *pCode, *pError;
//   
//   Shader newShader;
//   newShader.Name = name;
//   newShader.FilePath = filePath;
//   newShader.Types = typeFlags;
// 
//   /*=================
//       VERTEX SHADER  
//     =================*/
// 
//   if (typeFlags & ShaderType::VERTEX_SHADER)
//   {
//     hr = D3DCompileFromFile(filePath.c_str(), NULL, NULL, "mainVS", "vs_5_0", 0, 0, &pCode, &pError);
//     if (SUCCEEDED(hr))
//     {
//       DumpShaderCode(name + "VS.cso", pCode);
//       hr = g_d3d11Device->CreateVertexShader(pCode->GetBufferPointer(), pCode->GetBufferSize(), NULL, newShader.VertexShader.GetAddressOf());
//       if (FAILED(hr))
//         util::log::Warning("Failed to create vertex shader, HRESULT 0x%X", hr);
//       else
//       {
//         // For now use same input description for all shaders
//         D3D11_INPUT_ELEMENT_DESC inputDesc[2];
//         inputDesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
//         inputDesc[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0xC, D3D11_INPUT_PER_VERTEX_DATA, 0 };
// 
//         hr = g_d3d11Device->CreateInputLayout(inputDesc, 2, pCode->GetBufferPointer(), pCode->GetBufferSize(), newShader.InputLayout.GetAddressOf());
//         if (FAILED(hr))
//           util::log::Warning("Failed to create input layout for shader, HRESULT 0x%X", hr);
//       }
//     }
//     else
//     {
//       util::log::Warning("Failed to compile vertex shader from %S", filePath.c_str());
//       PrintErrorBlob(pError);
//     }
//   }
// 
//   /*===================
//       GEOMETRY SHADER
//     ===================*/
//   pError = nullptr;
// 
//   if (typeFlags & ShaderType::GEOMETRY_SHADER)
//   {
//     hr = D3DCompileFromFile(filePath.c_str(), NULL, NULL, "mainGS", "gs_5_0", 0, 0, &pCode, &pError);
//     if (SUCCEEDED(hr))
//     {
//       DumpShaderCode(name + "GS.cso", pCode);
//       hr = g_d3d11Device->CreateGeometryShader(pCode->GetBufferPointer(), pCode->GetBufferSize(), NULL, newShader.GeometryShader.GetAddressOf());
//       if (FAILED(hr))
//         util::log::Warning("Failed to create geometry shader, HRESULT 0x%X", hr);
//     }
//     else
//     {
//       util::log::Warning("Failed to compile geometry shader from %S", filePath.c_str());
//       PrintErrorBlob(pError);
//     }
//   }
// 
//   /*================
//       PIXEL SHADER
//     ================*/
// 
//   if (typeFlags & ShaderType::PIXEL_SHADER)
//   {
//     hr = D3DCompileFromFile(filePath.c_str(), NULL, NULL, "mainPS", "ps_5_0", 0, 0, &pCode, &pError);
//     if (SUCCEEDED(hr))
//     {
//       DumpShaderCode(name + "PS.cso", pCode);
//       hr = g_d3d11Device->CreatePixelShader(pCode->GetBufferPointer(), pCode->GetBufferSize(), NULL, newShader.PixelShader.GetAddressOf());
//       if (FAILED(hr))
//         util::log::Warning("Failed to create pixel shader, HRESULT 0x%X", hr);
//     }
//     else
//     {
//       util::log::Warning("Failed to compile pixel shader from %S", filePath.c_str());
//       PrintErrorBlob(pError);
//     }
//   }
// 
//   m_Shaders.emplace(name, newShader);
}

void ShaderStore::AddShaderFromResource(std::string const& name, int vsId, int gsId, int psId, int typeFlags)
{
  void* pData = nullptr;
  DWORD dwSize = 0;
  HRESULT hr;

  Shader newShader;
  newShader.Name = name;
  newShader.Types = typeFlags;

  if (typeFlags & VERTEX_SHADER)
  {
    if (util::GetResource(vsId, pData, dwSize))
    {
      hr = g_d3d11Device->CreateVertexShader(pData, dwSize, NULL, newShader.VertexShader.GetAddressOf());
      if (FAILED(hr))
        util::log::Warning("Failed to create vertex shader, HRESULT 0x%X", hr);
    }
  }

  if (typeFlags & GEOMETRY_SHADER)
  {
    if (util::GetResource(gsId, pData, dwSize))
    {
      hr = g_d3d11Device->CreateGeometryShader(pData, dwSize, NULL, newShader.GeometryShader.GetAddressOf());
      if (FAILED(hr))
        util::log::Warning("Failed to create geometry shader, HRESULT 0x%X", hr);
    }
  }

  if (typeFlags & PIXEL_SHADER)
  {
    if (util::GetResource(psId, pData, dwSize))
    {
      hr = g_d3d11Device->CreatePixelShader(pData, dwSize, NULL, newShader.PixelShader.GetAddressOf());
      if (FAILED(hr))
        util::log::Warning("Failed to create pixel shader, HRESULT 0x%X", hr);
    }
  }

  m_Shaders.emplace(name, newShader);
}

void ShaderStore::UseShader(std::string const& name)
{
  auto result = m_Shaders.find(name);
  if (result == m_Shaders.end())
  {
    util::log::Error("Shader %s does not exist", name.c_str());
    abort();
  }

  Shader& shader = result->second;
  if(shader.Types & GEOMETRY_SHADER)
    g_d3d11Context->GSSetShader(shader.GeometryShader.Get(), NULL, 0);
  if (shader.Types & PIXEL_SHADER)
    g_d3d11Context->PSSetShader(shader.PixelShader.Get(), NULL, 0);
  if (shader.Types & VERTEX_SHADER)
  {
    g_d3d11Context->VSSetShader(shader.VertexShader.Get(), NULL, 0);
    g_d3d11Context->IASetInputLayout(shader.InputLayout.Get());
  }
}

void ShaderStore::RecompileShaders()
{
  util::log::Write("Recompiling shaders");

  std::vector<Shader> shaders;
  for (auto& shader : m_Shaders)
    shaders.push_back(shader.second);

  m_Shaders.clear();
  for (auto& shader : shaders)
  {
    if (!shader.FilePath.empty())
      AddShaderFromFile(shader.Name, shader.FilePath, shader.Types);
    else
      m_Shaders.emplace(shader.Name, shader);
  }
}