#pragma once
#include "IComponent.h"

//! hitbox is the area that will hit the opponent
class StateComponent : public IComponent
{
public:
  StateComponent() : IComponent(nullptr) {}
  StateComponent(std::shared_ptr<Entity> owner) : IComponent(owner) {}
  
  //! is player on left side of the other player
  bool onLeftSide;

  //! if there were collisions this frame, what were they
  CollisionSide collision;

  //! Defender hit state information
  bool hitThisFrame = false;
  bool hitOnLeftSide = false;
  FrameData frameData;

  //! Attacker state information
  bool hitting = false;

  //!
  bool operator==(const StateComponent& other) const
  {
    return collision == other.collision && onLeftSide == other.onLeftSide && hitThisFrame == other.hitThisFrame && hitting == other.hitting;
  }

  bool operator!=(const StateComponent& other) const
  {
    return !(operator==(other));
  }

};
