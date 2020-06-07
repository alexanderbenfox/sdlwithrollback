#pragma once
#include "Entity.h"
#include "Transform.h"
#include <functional>

class ICollider : public IComponent
{
public:
  ICollider(std::shared_ptr<Entity> entity) : IComponent(entity) {}

  virtual void RegisterOnCollision(std::function<void(ICollider*)> onCollisionEvent)
  {
    _onCollisionCallbacks.push_back(onCollisionEvent);
  }

  virtual void OnCollision(ICollider* other)
  {
    for (auto& callback : _onCollisionCallbacks)
      callback(other);
  }

protected:
  std::vector<std::function<void(ICollider*)>> _onCollisionCallbacks;

};

//!
template <typename T>
class RectCollider : public ICollider
{
public:
  //!
  RectCollider(std::shared_ptr<Entity> entity) : ICollider(entity) {}
  //!
  void Init(Vector2<T> beg, Vector2<T> end);
  void MoveToTransform(const Transform& transform) { rect.CenterOnPoint(transform.position); }
  //!
  virtual void Draw() override;
  //!


  Rect<T> rect;
  Rect<T> unscaledRect;

  friend std::ostream& operator<<(std::ostream& os, const RectCollider& collider)
  {
    os << collider.rect;
    return os;
  }

  friend std::istream& operator>>(std::istream& is, RectCollider& collider)
  {
    is >> collider.rect;
    return is;
  }
};

typedef RectCollider<double> RectColliderD;

class Hitbox;
