#pragma once
#include "Systems/ISystem.h"
// for wall push
#include "Components/ActionComponents.h"
#include "Components/Rigidbody.h"

class WallPushSystem : public ISystem<Rigidbody, WallPushComponent, Transform>
{
public:
  static void DoTick(float dt);

};
