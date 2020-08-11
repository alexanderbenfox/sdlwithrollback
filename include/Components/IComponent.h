#pragma once
#include "Globals.h"
#include "Components/ComponentIDGenerator.h"
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
  IComponent(std::shared_ptr<Entity> owner) : _owner(owner) {}
  //! 
  virtual ~IComponent() = default;
  //! Gets the entity that owns the component
  virtual Entity* Owner() { return _owner.get(); }
  //! Draw the component for debug
  virtual void Draw() {}

protected:
  //! 
  std::shared_ptr<Entity> _owner;
  
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
