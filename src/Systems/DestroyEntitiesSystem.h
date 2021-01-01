#pragma once
#include "Core/ECS/ISystem.h"
#include "Managers/GameManagement.h"

#include "Core/Utility/DeferGuard.h"

struct DestroyOnSceneEnd : public IComponent {};

class DestroyEntitiesSystem : public ISystem<DestroyOnSceneEnd>
{
public:
  static void DoTick(float dt)
  {
    PROFILE_FUNCTION();
    DeferGuard guard;
    for(const EntityID& entity : Registered)
    {
      RunOnDeferGuardDestroy(entity, GameManager::Get().DestroyEntity(entity));
    }
  }
};
