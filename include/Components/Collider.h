#pragma once
#include "Entity.h"
#include "Transform.h"
#include <functional>

/*class ICollision
{
public:
  virtual ~ICollision() = 0;

};*/

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

  void SetStatic(bool staticObj) { _isStatic = staticObj; }
  const bool IsStatic() { return _isStatic; }

protected:
  std::vector<std::function<void(ICollider*)>> _onCollisionCallbacks;
  bool _isStatic;
  bool _debug;

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
  void MoveToTransform(const Transform& transform)
  {
    rect = Rect<T>(Vector2<T>((T)transform.position.x, (T)transform.position.y), Vector2<T>(transform.position.x + rect.Width(), transform.position.y + rect.Height()));
  }
  //!
  virtual void Draw() override;
  //!
  Rect<T> rect;

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

template <> struct ComponentTraits<RectColliderD>
{
  static const uint64_t GetSignature() { return 1 << 4;}
};


class Hitbox : public RectColliderD
{
public:
  //!
  Hitbox(std::shared_ptr<Entity> entity) : RectColliderD(entity) {}
};
template <> struct ComponentTraits<Hitbox>
{
  static const uint64_t GetSignature() { return 1 << 7; }
};