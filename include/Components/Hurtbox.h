#pragma once
#include "Components/Collider.h"
#include "DebugGUI/DebugItem.h"

//! hurtbox is the area that you can take damage from an enemy attack
class Hurtbox : public RectColliderD, public DebugItem
{
public:
  //!
  Hurtbox(std::shared_ptr<Entity> entity);
  //! Shows option to draw debug rectangles around hurtbox
  virtual void OnDebug() override;

};

template <> struct ComponentInitParams<Hurtbox>
{
  Vector2<double> size;
  static void Init(Hurtbox& component, const ComponentInitParams<Hurtbox>& params)
  {
    component.rect = Rect<double>(0, 0, params.size.x, params.size.y);
  }
};
