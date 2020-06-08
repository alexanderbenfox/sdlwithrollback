#pragma once
#include "Components/Collider.h"
#include "DebugGUI/DebugItem.h"

//! hurtbox is the area that you can take damage from an enemy attack
class Hurtbox : public RectColliderD, public DebugItem
{
public:
  //!
  Hurtbox(std::shared_ptr<Entity> entity);
  //! Do we put health here?
  int hp = 100;

  virtual void OnDebug() override;

};
