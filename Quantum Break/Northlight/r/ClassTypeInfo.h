#pragma once

namespace r
{
  class ClassTypeInfo
  {
  public:
    virtual const char* GetTypeName();
    virtual int GetCRCSum();
    virtual int GetClassId();
    virtual ClassTypeInfo* GetBaseClass();
    virtual void Func5();
    virtual void Func6();
    virtual void Func7();
    virtual __int64 Create();

    ClassTypeInfo* m_current;
    ClassTypeInfo* m_next;

  };
}