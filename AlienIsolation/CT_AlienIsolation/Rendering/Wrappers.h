#pragma once

class GeometricWrapper
{
public:
  virtual void* GetShape() { return nullptr; }
};

class CMOWrapper
{
public:
  virtual void* GetModel() { return nullptr; }
};

class ImageWrapper
{
public:
  virtual void* GetView() { return nullptr; }
};