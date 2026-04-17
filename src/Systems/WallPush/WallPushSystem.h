#pragma once
#include "Core/ECS/ISystem.h"
// for wall push
#include "Components/ActionComponents.h"
#include "Components/Rigidbody.h"

class WallPushSystem : public ISystem<Rigidbody, WallPushComponent, Transform>
{
public:
  static void Check(Entity* entity)
  {
    bool wasMember = Registered.count(entity->GetID());
    ISystem::Check(entity);
    bool isMember = Registered.count(entity->GetID());
    if (wasMember && !isMember)
    {
      if (ComponentArray<Rigidbody>::Get().HasComponent(entity->GetID()))
        ComponentArray<Rigidbody>::Get().GetComponent(entity->GetID()).velocity.x = 0;
    }
  }

  static void DoTick(float dt);

};
