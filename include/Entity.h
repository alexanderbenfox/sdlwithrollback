#pragma once
#include <unordered_map>
#include <typeindex>
#include <memory>

template <typename T>
class Vector2
{
public:
  Vector2<T>() : x(0), y(0) {}
  Vector2<T>(T x, T y) : x(x), y(y) {}

  T x;
  T y;

  void operator+=(const Vector2<T>& other) { x += other.x; y += other.y; }
};

template <typename T>
Vector2<T> operator+(const Vector2<T>& lhs, const Vector2<T>& rhs) { return Vector2<T>(lhs.x + rhs.x, lhs.y + rhs.y); }

template <typename T>
Vector2<T> operator*(const Vector2<T>& lhs, T rhs) { return Vector2<T>(lhs.x * rhs, lhs.y * rhs); }

template <typename T>
Vector2<T> operator*(T lhs, const Vector2<T>& rhs) { return Vector2<T>(lhs * rhs.x, lhs * rhs.y); }

template <typename T, typename U>
Vector2<T> operator*(const Vector2<T>& lhs, U rhs) { return Vector2<T>(static_cast<T>(static_cast<U>(lhs.x) * rhs), static_cast<T>(static_cast<U>(lhs.y) * rhs)); }

struct Transform
{
  Transform() : position(Vector2<int>(0, 0)), scale(Vector2<float>(1.0f, 1.0f)), rotation(Vector2<float>(0.0f, 0.0f)) {}
  Vector2<int> position;
  Vector2<float> scale;
  Vector2<float> rotation;
};

class Entity;

class IComponent
{
public:
  IComponent(Entity* owner) : _owner(owner) {}
  //virtual ~IComponent() = 0;
  virtual void Update(float dt) {}// = 0;
protected:
  Entity* _owner;
  
};

//! Entity has componentsw
class Entity
{
public:
  Entity() {}

  virtual void Update(float dt);

  template <typename T = IComponent> 
  T* GetComponent();

  template <typename T = IComponent>
  void AddComponent();

  template <typename T = IComponent>
  void RemoveComponent();

  Transform transform;

protected:
  std::unordered_map<std::type_index, IComponent*> _components;
};

template <typename T>
inline T* Entity::GetComponent()
{
  if (_components.find(std::type_index(typeid(T))) != _components.end())
    return static_cast<T*>(_components[std::type_index(typeid(T))]);
  else return nullptr;
}

template <typename T>
inline void Entity::AddComponent()
{
  if (_components.find(std::type_index(typeid(T))) == _components.end())
  {
    _components.insert(std::make_pair(std::type_index(typeid(T)), ComponentManager<T>::Get().Create(this)));
  }
}

template <typename T>
inline void Entity::RemoveComponent()
{
  if (_components.find(std::type_index(typeid(T))) != _components.end())
    _components.erase(std::type_index(typeid(T)));
}
