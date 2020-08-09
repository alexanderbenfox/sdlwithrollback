#pragma once
#include "Systems/ISystem.h"
#include "Systems/WallPush/WallPushComponent.h"

class WallPushSystem : public ISystem<Rigidbody, WallPushComponent, Transform>
{
public:
  static void DoTick(float dt);

};
