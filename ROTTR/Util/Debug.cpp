#include "Util.h"
#include "../imgui/imgui.h"

#include "../Frostbite/Core/TypeInfo.hpp"
#include <boost/assign.hpp>
#include <map>
#include <iomanip>
#include <sstream>

using namespace boost::assign;

namespace
{
  char* sClassName = new char[100]{ '\0' };
  char* sClassInfo = new char[1000]{ '\0' };
  size_t szClassInfoString = 1000;
  std::map<std::string, std::string> FBTypeMap = map_list_of
  ("Int16", "int16_t")
    ("Int32", "int")
    ("Int64", "__int64")
    ("Uint16", "uint16_t")
    ("Uint32", "unsigned int")
    ("Float32", "float")
    ("Boolean", "bool")
    ("Vec2", "XMFLOAT2")
    ("Vec3", "XMFLOAT4")
    ("Vec4", "XMFLOAT4");
}

static std::string ToCType(std::string const& fbType)
{
  auto result = FBTypeMap.find(fbType);
  if (result == FBTypeMap.end())
    return fbType;

  return result->second;
}

static std::string ToHex(int i)
{
  std::stringstream stream;
  stream << "0x" << std::hex << std::setw(4) << std::setfill('0') << i;
  return stream.str();
}

static void DumpTypeInfo(std::string const& name)
{
  fb::TypeInfo* pTypeInfo = fb::TypeInfo::GetByName(name.c_str());
  if (!pTypeInfo)
  {
    util::log::Write("TypeInfo for %s does not exist", name.c_str());
    return;
  }

  const fb::MemberInfo::MemberInfoData* pMemberInfo = pTypeInfo->GetMemberInfoData();

  int typeCategory = (pMemberInfo->Flags >> 2) & 0x3;
  int typeEnum = pMemberInfo->Flags & 0x1F;
  std::string InfoString = "";
  std::map<int, std::string> FieldMap;

  if (typeEnum == 21) // Class
  {
    const fb::ClassInfo::ClassInfoData* pClassInfoData = reinterpret_cast<const fb::ClassInfo::ClassInfoData*>(pMemberInfo);

    InfoString += "class " + name;
    if (typeCategory == 1 && pClassInfoData->SuperClass)
      InfoString += " : public " + std::string(pClassInfoData->SuperClass->GetMemberInfoData()->Name);

    InfoString += "\n{\n";
    for (int i = 0; i < pClassInfoData->BasicInfo.FieldCount; ++i)
    {
      fb::FieldInfo::FieldInfoData& field = pClassInfoData->Fields[i];

      std::string FieldString = "\t" + ToCType(field.FieldType->GetMemberInfoData()->Name);
      FieldString += " m_" + std::string(field.Name) + "; // +" + ToHex(field.Offset);
      FieldMap.emplace(field.Offset, FieldString);
    }
  }
  else if (typeEnum == 9)
  {
    const fb::EnumTypeInfo::EnumTypeInfoData* pEnumInfoData = reinterpret_cast<const fb::EnumTypeInfo::EnumTypeInfoData*>(pMemberInfo);

    InfoString += "enum class " + name;
    InfoString += "\n{\n";

    for (int i = 0; i < pEnumInfoData->BasicInfo.FieldCount; ++i)
    {
      fb::FieldInfo::FieldInfoData& field = pEnumInfoData->Fields[i];
      int enumCount = reinterpret_cast<int>(field.FieldType);

      std::string FieldString = "\t" + std::string(field.Name) + " = " + std::to_string(enumCount) + ",";
      FieldMap.emplace(enumCount, FieldString);
    }
  }
  else
    util::log::Warning("Trying to dump type that isn't a class or enum");

  for (auto& field : FieldMap)
    InfoString += field.second + "\n";
  
  InfoString += "};";

  sClassInfo = new char[InfoString.size() + 1];
  memcpy(sClassInfo, InfoString.c_str(), InfoString.size() + 1);
  szClassInfoString = InfoString.size() + 1;
}

void util::debug::DrawTypeDumper()
{
  ImGui::Begin("TypeInfo dumper");

  ImGui::Columns(2, 0, false);
  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 10);

  ImGui::InputText("Class name", sClassName, 100);
  if (ImGui::Button("Dump"))
    DumpTypeInfo(sClassName);

  ImGui::InputTextMultiline("##ClassInfo", sClassInfo, szClassInfoString, ImVec2(500,350), ImGuiInputTextFlags_ReadOnly);

  ImGui::End();

  //util::debug::DrawReflectionTest();
}

///////////////////////////////
///////////////////////////////
////                       ////
////    REFLECTION TEST    ////
////                       ////
///////////////////////////////
///////////////////////////////

class IDataController
{
public:
  IDataController() {};
  ~IDataController() {};

  virtual void Draw() = 0;

protected:
  void* m_pData;
  const char* m_Name;

public:
  IDataController(IDataController const&) = delete;
  void operator=(IDataController const&) = delete;
};

enum PrimitiveType
{
  Int32,
  Int16,
  Float32,
  Boolean
};

class PrimitiveController : public IDataController
{
public:
  PrimitiveController(const char* name, void* pInstance, int offset, PrimitiveType type);
  ~PrimitiveController() {};

  void Draw();

private:
  PrimitiveType m_Type;
};

PrimitiveController::PrimitiveController(const char* name, void* pInstance, int offset, PrimitiveType type)
{
  m_Name = name;
  m_pData = reinterpret_cast<void*>((__int64)pInstance + offset);
  m_Type = type;
}

void PrimitiveController::Draw()
{
  switch (m_Type)
  {
  case Int32:
    ImGui::InputInt(m_Name, reinterpret_cast<int*>(m_pData));
    break;
  case Float32:
    ImGui::InputFloat(m_Name, reinterpret_cast<float*>(m_pData));
    break;
  case Boolean:
    ImGui::Checkbox(m_Name, reinterpret_cast<bool*>(m_pData));
  }
}

class FloatController : public IDataController
{
public:
  FloatController(const char* name, float* pFloat);
  ~FloatController() {};

  void Draw();
};

FloatController::FloatController(const char* name, float* pFloat)
{
  m_pData = pFloat;
  m_Name = name;
}

void FloatController::Draw()
{
  ImGui::InputFloat(m_Name, reinterpret_cast<float*>(m_pData));
}

class EnumController : public IDataController
{
public:
  EnumController(const char* name, void* pInstance, int offset, fb::EnumTypeInfo* pEnumType);
  ~EnumController() {};

  void Draw();

private:
  int m_EnumCount;
  const char** m_EnumStrings;
};

EnumController::EnumController(const char* name, void* pInstance, int offset, fb::EnumTypeInfo* pEnumType)
{
  m_Name = name;
  m_pData = reinterpret_cast<void*>((__int64)pInstance + offset);
  
  const fb::EnumTypeInfo::EnumTypeInfoData* pEnumInfo = reinterpret_cast<const fb::EnumTypeInfo::EnumTypeInfoData*>(pEnumType->GetMemberInfoData());
  m_EnumCount = pEnumInfo->BasicInfo.FieldCount;
  m_EnumStrings = new const char*[pEnumInfo->BasicInfo.FieldCount];

  for (int i = 0; i < pEnumInfo->BasicInfo.FieldCount; ++i)
    m_EnumStrings[i] = pEnumInfo->Fields[i].Name;
}

void EnumController::Draw()
{
  ImGui::Combo(m_Name, reinterpret_cast<int*>(m_pData), m_EnumStrings, m_EnumCount);
}

class RefController : public IDataController
{
public:
  RefController(const char* name, void* pInstance, int offset, fb::ClassInfo* pRefType);
  ~RefController() {};

  void Draw();

private:
  void* m_CurrentRef;
  const char* m_RefName;
  int m_szName;
};

RefController::RefController(const char* name, void* pInstance, int offset, fb::ClassInfo* pRefType)
{
  m_Name = name;
  m_pData = reinterpret_cast<void*>((__int64)pInstance + offset);
  m_CurrentRef = *(void**)(m_pData);
  if (m_CurrentRef)
    m_RefName = *(const char**)((__int64)m_CurrentRef + 0x10);
  else
    m_RefName = "None\0";

  m_szName = strlen(m_RefName);
}

void RefController::Draw()
{
  ImGui::InputText(m_Name, (char*)m_RefName, m_szName, ImGuiInputTextFlags_ReadOnly);
}

std::map<int, IDataController*> m_Controllers;
void* m_Instance;

static void ReflectToImGui(std::string const& name)
{
  fb::TypeInfo* pTypeInfo = fb::TypeInfo::GetByName(name.c_str());
  if (!pTypeInfo)
  {
    util::log::Write("TypeInfo for %s does not exist", name.c_str());
    return;
  }

  const fb::MemberInfo::MemberInfoData* pMemberInfo = pTypeInfo->GetMemberInfoData();
  int typeEnum = pMemberInfo->Flags & 0x1F;
  if (typeEnum != 21)
  {
    util::log::Write("%s is not a class type", name.c_str());
    return;
  }

  fb::ClassInfo* pClassInfo = reinterpret_cast<fb::ClassInfo*>(pTypeInfo);
  auto pClassData = reinterpret_cast<const fb::ClassInfo::ClassInfoData*>(pMemberInfo);

  m_Instance = new char[pClassData->BasicInfo.TotalSize];
  memcpy(m_Instance, (void*)pClassInfo->m_DefaultInstance, pClassData->BasicInfo.TotalSize);

  const fb::ClassInfo::ClassInfoData* pClassInfoData = reinterpret_cast<const fb::ClassInfo::ClassInfoData*>(pMemberInfo);
  m_Controllers.clear();
  for (int i = 0; i < pClassInfoData->BasicInfo.FieldCount; ++i)
  {
    fb::FieldInfo::FieldInfoData& field = pClassInfoData->Fields[i];
    const fb::MemberInfo::MemberInfoData* pFieldMemberInfo = field.FieldType->GetMemberInfoData();

    int typeCategory = (pFieldMemberInfo->Flags >> 4) & 0x1F;
    if (typeCategory >= 10 && typeCategory <= 20)
    {
      PrimitiveType type;
      if (strcmp("Int32", pFieldMemberInfo->Name) == 0)
        type = PrimitiveType::Int32;
      else if (strcmp("Int16", pFieldMemberInfo->Name) == 0)
        type = PrimitiveType::Int16;
      else if (strcmp("Float32", pFieldMemberInfo->Name) == 0)
        type = PrimitiveType::Float32;
      else if (strcmp("Boolean", pFieldMemberInfo->Name) == 0)
        type = PrimitiveType::Boolean;
      else
      {
        util::log::Write("%s is not a valid primitive type", pFieldMemberInfo->Name);
        continue;
      }
      m_Controllers.emplace(field.Offset, new PrimitiveController(field.Name, m_Instance, field.Offset, type));
    }
    else if (typeCategory == 8)
    {
      m_Controllers.emplace(field.Offset, new EnumController(field.Name, m_Instance, field.Offset, reinterpret_cast<fb::EnumTypeInfo*>(field.FieldType)));
    }
    else if (typeCategory == 3)
    {
      m_Controllers.emplace(field.Offset, new RefController(field.Name, m_Instance, field.Offset, reinterpret_cast<fb::ClassInfo*>(field.FieldType)));
    }
    else
      util::log::Write("TypeCategory %d not supported", typeCategory);
  }

}

void util::debug::DrawReflectionTest()
{
  ImGui::Begin("Reflection testing");

  ImGui::Columns(2, 0, false);
  ImGui::NextColumn();
  ImGui::SetColumnOffset(-1, 10);

  ImGui::InputText("Class name", sClassName, 100);
  if (ImGui::Button("Reflect"))
    ReflectToImGui(sClassName);

  for (auto& entry : m_Controllers)
  {
    IDataController* pDataController = entry.second;
    pDataController->Draw();
  }

  ImGui::End();
}
