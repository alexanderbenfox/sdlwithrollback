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

template <typename T, typename U>
Vector2<T> operator*(const Vector2<T>& lhs, U rhs) { return Vector2<T>(lhs.x * static_cast<T>(rhs), lhs.y * static_cast<T>(rhs)); }

struct Transform
{
  Transform() : position(Vector2<int>(0, 0)), scale(Vector2<float>(1.0f, 1.0f)), rotation(Vector2<float>(0.0f, 0.0f)) {}
  Vector2<int> position;
  Vector2<float> scale;
  Vector2<float> rotation;
};

class IComponent
{
public:
  //virtual ~IComponent() = 0;

  virtual void Update(Transform& transform, float dt) = 0;
  virtual void PushToRenderer(const Transform& transform) = 0;
};

//! Entity has componentsw
class Entity
{
public:
  Entity() {}

  virtual void Update(float dt);

  virtual void PushToRenderer();

  template <typename T = IComponent> 
  T* GetComponent();

  template <typename T = IComponent>
  void AddComponent();

  template <typename T = IComponent>
  void RemoveComponent();

  Transform transform;

protected:
  std::unordered_map<std::type_index, std::shared_ptr<IComponent>> _components;
};

template <typename T>
inline T* Entity::GetComponent()
{
  if (_components.find(std::type_index(typeid(T))) != _components.end())
    return static_cast<T*>(_components[std::type_index(typeid(T))].get());
  else return nullptr;
}

template <typename T>
inline void Entity::AddComponent()
{
  if (_components.find(std::type_index(typeid(T))) == _components.end())
  {
    _components.insert(std::make_pair(std::type_index(typeid(T)), new T()));
  }
}

template <typename T>
inline void Entity::RemoveComponent()
{
  if (_components.find(std::type_index(typeid(T))) != _components.end())
    _components.erase(std::type_index(typeid(T)));
}
