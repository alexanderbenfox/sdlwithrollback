#pragma once
#include "Core/ECS/ISystem.h"
#include "Managers/GameManagement.h"

struct DestroyOnSceneEnd : public IComponent {};

class DestroyEntitiesSystem : public ISystem<DestroyOnSceneEnd>
{
public:
  static void DoTick(float dt)
  {
    DeferScopeGuard guard;
    for(const EntityID& entity : Registered)
    {
      defer(entity, GameManager::Get().DestroyEntity(entity););
    }
  }
};
