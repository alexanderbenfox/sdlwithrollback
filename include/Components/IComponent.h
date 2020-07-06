#pragma once
#include <memory>

#include <iostream>
#include "Utils.h"
#include "ComponentConst.h"

class Entity;

// IDEA: Split up components into their data and functions that change that data
// Based on reducers in redux??


template <typename T>
struct ComponentInitParams
{
  static void Init(T& component, const ComponentInitParams<T>& params) {}
};

class IComponent
{
public:
  IComponent(std::shared_ptr<Entity> owner) : _owner(owner) {}
  //!
  virtual ~IComponent() {}
  //!
  virtual void OnFrameBegin() {}
  //!
  virtual void OnFrameEnd() {}
  //!
  virtual void PreUpdate() {}
  //!
  virtual void PostUpdate() {}
  //!
  virtual void Draw() {}

protected:
  //! 
  std::shared_ptr<Entity> _owner;
  
};

template <typename T>
struct ComponentTraits
{
  static int ID;
  static const uint64_t GetSignature() { return 1LL << ID; }
};

template <class T>
int ComponentTraits<T>::ID = ConstComponentIDGenerator::NextID();
