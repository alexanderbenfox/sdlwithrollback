#pragma once
#include <unordered_map>
#include <typeindex>
#include <memory>

#include "Geometry.h"
#include "Components/ComponentManager.h"
#include "Utils.h"

static int EntityID = 0;

class IDebuggable
{
public:
  virtual ~IDebuggable() {}
  virtual void ParseCommand(const std::string& command) = 0;
  virtual std::string GetIdentifier() = 0;
};

//______________________________________________________________________________
//!
struct Transform
{
  Transform() : position(Vector2<float>(0.0f, 0.0f)), scale(Vector2<float>(1.0f, 1.0f)), rotation(Vector2<float>(0.0f, 0.0f)) {}
  Vector2<float> position;
  Vector2<float> scale;
  Vector2<float> rotation;

  friend std::ostream& operator<<(std::ostream& os, const Transform& transform);
  friend std::istream& operator>>(std::istream& is, Transform& transform);
};

//______________________________________________________________________________
//! Entity is the root of the component tree containing all of the positional information for the game object
class Entity : public IDebuggable
{
public:
  //! Increment creation id counter
  Entity() : _creationId(EntityID++) {}
  //! Updates all components attached to the entity
  virtual void Update(float dt);
  //! Retrieves the components of type specified or nullptr if there is no component of that type present
  template <typename T = IComponent> 
  std::shared_ptr<T> GetComponent();
  //! Adds the component of the type specified to this entity
  template <typename T = IComponent>
  void AddComponent();
  //! Removes component of type specified from the entity
  template <typename T = IComponent>
  void RemoveComponent();

  //! Transform attached to the object defining the position, scale, and rotation of the object
  Transform transform;
  //!
  virtual void ParseCommand(const std::string& command) override;
  //!
  virtual std::string GetIdentifier() override;

  void SetScale(Vector2<float> scale);

protected:
  //! Pointers to all components attached to the object. The component objects exist in their respective manager singleton objects
  std::unordered_map<std::type_index, std::shared_ptr<IComponent>> _components;
  //!
  int _creationId;

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
    _components.insert(std::make_pair(std::type_index(typeid(T)), ComponentManager<T>::Get().Create(std::shared_ptr<Entity>(this))));
  }
}

//______________________________________________________________________________
template <typename T>
inline void Entity::RemoveComponent()
{
  if (_components.find(std::type_index(typeid(T))) != _components.end())
    _components.erase(std::type_index(typeid(T)));
}
