#pragma once
#include "Components/IComponent.h"
#include "Components/Rigidbody.h"

//! Component for pushing player away from other player when pressuring on the wall
struct WallPushComponent : public IComponent
{
  WallPushComponent(std::shared_ptr<Entity> entity) : IComponent(entity) {}
  ~WallPushComponent()
  {
    if (_owner->GetComponent<Rigidbody>())
      _owner->GetComponent<Rigidbody>()->_vel.x = 0;
  }
  float pushAmount;
  float amountPushed = 0.0f;
  float velocity;
};
