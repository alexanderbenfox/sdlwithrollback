#pragma once
#include "Components/Collider.h"

//! hurtbox is the area that you can take damage from an enemy attack
class Hurtbox : public RectColliderD
{
public:
  //!
  Hurtbox(std::shared_ptr<Entity> entity);
  ~Hurtbox();
  //! Do we put health here?
  int hp = 100;
  int debugID = -1;

};
