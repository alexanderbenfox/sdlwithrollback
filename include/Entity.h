#pragma once
#include <unordered_map>
#include <typeindex>
#include <memory>

#include "Core/Math/Vector2.h"
#include "Components/ComponentManager.h"
#include "Utils.h"

template <typename T, typename... Rest>
bool constexpr all_base_of_bigboysss()
{
  return (std::is_base_of_v<T, Rest> && ...);
}

static int EntityID = 0;

class IDebuggable
{
public:
  virtual ~IDebuggable() {}
  virtual void ParseCommand(const std::string& command) = 0;
  virtual std::string GetIdentifier() = 0;
};

//______________________________________________________________________________
//! Entity is the root of the component tree containing all of the positional information for the game object
class Entity : public IDebuggable, public std::enable_shared_from_this<Entity>
{
public:
  //! Increment creation id counter
  Entity() : ComponentBitFlag(0x0), _creationId(EntityID++) {}
  //! Retrieves the components of type specified or nullptr if there is no component of that type present
  template <typename T = IComponent> 
  std::shared_ptr<T> GetComponent();
  //! Adds the component of the type specified to this entity
  template <typename T = IComponent>
  void AddComponent();
  //! Removes component of type specified from the entity
  template <typename T = IComponent>
  void RemoveComponent();
  //!
  template <typename ... T>
  std::tuple<std::add_pointer_t<T>...> MakeComponentTuple();

  //! Transform attached to the object defining the position, scale, and rotation of the object
  // Transform transform;
  //!
  virtual void ParseCommand(const std::string& command) override;
  //!
  virtual std::string GetIdentifier() override;

  void SetScale(Vector2<float> scale);

  int GetID() const {return _creationId;}

  uint64_t ComponentBitFlag;

protected:
  //! Pointers to all components attached to the object. The component objects exist in their respective manager singleton objects
  std::unordered_map<std::type_index, std::shared_ptr<IComponent>> _components;
  //!
  int _creationId;
  //!
  template <typename T>
  void SetPointerElement(T*& element) { element = GetComponent<T>().get(); }
  //!
  static void CheckAgainstSystems(Entity* entity);

};

//______________________________________________________________________________
template <typename T>
inline std::shared_ptr<T> Entity::GetComponent()
{
  if (_components.find(std::type_index(typeid(T))) != _components.end())
    return std::static_pointer_cast<T>(_components[std::type_index(typeid(T))]);
  else return std::shared_ptr<T>(nullptr);
}

//______________________________________________________________________________
template <typename T>
inline void Entity::AddComponent()
{
  if (_components.find(std::type_index(typeid(T))) == _components.end())
  {
    ComponentBitFlag |= ComponentTraits<T>::GetSignature();
    _components.insert(std::make_pair(std::type_index(typeid(T)), ComponentManager<T>::Get().Create(std::shared_ptr<Entity>(shared_from_this()))));
    // see if this needs to be added to the system
    CheckAgainstSystems(this);
  }
}

//______________________________________________________________________________
template <typename T>
inline void Entity::RemoveComponent()
{
  if (_components.find(std::type_index(typeid(T))) != _components.end())
  {
    ComponentBitFlag &= ~ComponentTraits<T>::GetSignature();
    ComponentManager<T>::Get().Erase(GetComponent<T>());
    _components.erase(std::type_index(typeid(T)));

    // see if this needs to be added to the system
    CheckAgainstSystems(this);
  }
}

//______________________________________________________________________________
template <typename ... T>
inline std::tuple<std::add_pointer_t<T>...> Entity::MakeComponentTuple()
{
  std::tuple<std::add_pointer_t<T>...> tuple;
  (SetPointerElement<T>(std::get<T*>(tuple)), ...);
  return tuple;
}
