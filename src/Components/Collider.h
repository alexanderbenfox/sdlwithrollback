#pragma once
#include "Core/ECS/Entity.h"
#include "Components/Transform.h"
#include <functional>

class ICollider : public IComponent
{
public:
  virtual void OnCollision(const EntityID& entity, ICollider* other) = 0;

};

//!
template <typename T>
class RectCollider : public ICollider
{
public:
  //!
  virtual void OnCollision(const EntityID& entity, ICollider* other) override {}
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
