#pragma once
#include "Entity.h"
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

  virtual void Update(float dt) override {}

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
  //!
  virtual void Update(float dt) override;
  //!
  Rect<T> rect;

};

typedef RectCollider<double> RectColliderD;
