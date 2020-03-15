#pragma once
#include "Collider.h"
#include "IComponent.h"

//! hitbox is the area that will hit the opponent
class Hitbox : public RectColliderD
{
public:
  //!
  Hitbox(std::shared_ptr<Entity> entity) : RectColliderD(entity) {}
  //!
  FrameData frameData;

private:

};
template <> struct ComponentTraits<Hitbox>
{
  static const uint64_t GetSignature() { return 1 << 8; }
};