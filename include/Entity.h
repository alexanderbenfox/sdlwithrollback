#pragma once
#include <unordered_map>
#include <typeindex>
#include <memory>

#include "Geometry.h"
#include "Components/ComponentManager.h"

struct Transform
{
  Transform() : position(Vector2<float>(0.0f, 0.0f)), scale(Vector2<float>(1.0f, 1.0f)), rotation(Vector2<float>(0.0f, 0.0f)) {}
  Vector2<float> position;
  Vector2<float> scale;
  Vector2<float> rotation;
};

//! Entity has componentsw
class Entity
{
public:
  Entity() {}

  virtual void Update(float dt);

  template <typename T = IComponent> 
  std::shared_ptr<T> GetComponent();

  template <typename T = IComponent>
  void AddComponent();

  template <typename T = IComponent>
  void RemoveComponent();

  Transform transform;

protected:
  std::unordered_map<std::type_index, std::shared_ptr<IComponent>> _components;
};

template <typename T>
inline std::shared_ptr<T> Entity::GetComponent()
{
  if (_components.find(std::type_index(typeid(T))) != _components.end())
    return std::static_pointer_cast<T>(_components[std::type_index(typeid(T))]);
  else return std::shared_ptr<T>(nullptr);
}

template <typename T>
inline void Entity::AddComponent()
{
  if (_components.find(std::type_index(typeid(T))) == _components.end())
  {
    _components.insert(std::make_pair(std::type_index(typeid(T)), ComponentManager<T>::Get().Create(std::shared_ptr<Entity>(this))));
  }
}

template <typename T>
inline void Entity::RemoveComponent()
{
  if (_components.find(std::type_index(typeid(T))) != _components.end())
    _components.erase(std::type_index(typeid(T)));
}
