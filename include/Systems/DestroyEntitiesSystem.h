#pragma once
#include "Systems/ISystem.h"
#include "GameManagement.h"

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
