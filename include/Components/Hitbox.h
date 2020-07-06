#pragma once
#include "Collider.h"
#include "IComponent.h"

//! hitbox is the area that will hit the opponent
class Hitbox : public RectColliderD
{
public:
  //!
  Hitbox(std::shared_ptr<Entity> entity) : hit(false), RectColliderD(entity) {}
  //!
  HitData hitData;

  bool hit = false;

  bool travelWithTransform = false;

  bool destroyOnHit = false;

  virtual void OnCollision(ICollider* collider) override
  {
    if (destroyOnHit)
    {
      _owner->DestroySelf();
    }
  }

};

template <> struct ComponentInitParams<Hitbox>
{
  Vector2<double> size;
  HitData hData;
  bool travelWithTransform;
  bool destroyOnHit;
  static void Init(Hitbox& component, const ComponentInitParams<Hitbox>& params)
  {
    component.rect = Rect<double>(0, 0, params.size.x, params.size.y);
    component.hitData = params.hData;
    component.travelWithTransform = params.travelWithTransform;
    component.destroyOnHit = params.destroyOnHit;
  }
};
