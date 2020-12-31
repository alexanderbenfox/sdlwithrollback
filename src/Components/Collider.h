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
class RectCollider : public ICollider, ISerializable
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

  virtual void Serialize(std::ostream& os) const override
  {
    os << rect;
  }

  virtual void Deserialize(std::istream& is) override
  {
    is >> rect;
  }

  std::string Log() override
  {
    std::stringstream ss;
    ss << "RectCollider\n";
    ss << "\tRect x = " << rect.beg.x << " y = " << rect.beg.y << " w = " << rect.Width() << " h = " << rect.Height() << "\n";
    return ss.str();
  }

protected:
  bool _drawDebug = true;

};

typedef RectCollider<double> RectColliderD;

class Hitbox;
