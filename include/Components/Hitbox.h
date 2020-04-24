#pragma once
#include "Collider.h"
#include "IComponent.h"

//! hitbox is the area that will hit the opponent
class Hitbox : public RectColliderD
{
public:
  //!
  Hitbox(std::shared_ptr<Entity> entity) : hit(false), strikeVector(0, 0), RectColliderD(entity) {}
  //!
  FrameData frameData;

  bool hit = false;
  //! indicates that the hit happened this frame
  bool hitting = false;

  Vector2<int> strikeVector;

private:

};
