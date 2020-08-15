#pragma once
#include "Globals.h"
#include "Core/ECS/ComponentIDGenerator.h"
#include <memory>
#include <bitset>

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
  //! 
  virtual ~IComponent() = default;
  //!
  virtual void OnAdd(const EntityID& entity) {}
  virtual void OnRemove(const EntityID& entity) {}
  //! Draw the component for debug
  virtual void Draw() {}
  
};

template <typename T>
struct ComponentTraits
{
  static int ID;
  static bool ID_Initialized;
  static std::bitset<MAX_COMPONENTS> signature;

  static const std::bitset<MAX_COMPONENTS>& GetSignature() { return signature; }
};

template <class T>
int ComponentTraits<T>::ID = 0;

template <class T>
bool ComponentTraits<T>::ID_Initialized = false;

template <class T>
std::bitset<MAX_COMPONENTS> ComponentTraits<T>::signature = ComponentIDGenerator::GenerateBitFlag(ComponentTraits<T>::ID_Initialized, ComponentTraits<T>::ID, ComponentTraits<T>::signature);
