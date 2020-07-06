#pragma once
#include "Entity.h"
#include "Transform.h"
#include <functional>

class ICollider : public IComponent
{
public:
  ICollider(std::shared_ptr<Entity> entity) : IComponent(entity) {}

  virtual void OnCollision(ICollider* other) = 0;

};

//!
template <typename T>
class RectCollider : public ICollider
{
public:
  //!
  RectCollider(std::shared_ptr<Entity> entity) : ICollider(entity) {}
  //!
  virtual void OnCollision(ICollider* other) override {}
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

protected:
  bool _drawDebug = true;

};

typedef RectCollider<double> RectColliderD;

class Hitbox;
