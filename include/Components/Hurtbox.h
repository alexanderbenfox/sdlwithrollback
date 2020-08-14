#pragma once
#include "Components/Collider.h"
#include "DebugGUI/DebugItem.h"

//! hurtbox is the area that you can take damage from an enemy attack
struct Hurtbox : public RectColliderD {};

template <> struct ComponentInitParams<Hurtbox>
{
  Vector2<double> size;
  static void Init(Hurtbox& component, const ComponentInitParams<Hurtbox>& params)
  {
    component.rect = Rect<double>(0, 0, params.size.x, params.size.y);
  }
};
