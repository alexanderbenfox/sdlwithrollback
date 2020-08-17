#pragma once
#include "Globals.h"
#include "Core/ECS/ComponentIDGenerator.h"

#include <memory>
#include <bitset>

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
  //! 
  virtual ~IComponent() = default;
  //!
  virtual void OnAdd(const EntityID& entity) {}
  virtual void OnRemove(const EntityID& entity) {}
  //! Draw the component for debug
  virtual void Draw() {}
  
};
