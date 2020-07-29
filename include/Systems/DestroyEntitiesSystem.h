#pragma once
#include "Systems/ISystem.h"
#include "GameManagement.h"

struct DestroyOnSceneEnd : public IComponent
{
  DestroyOnSceneEnd(std::shared_ptr<Entity> e) : IComponent(e) {}
  std::shared_ptr<Entity> Entity() { return _owner; }
};

class DestroyEntitiesSystem : public ISystem<DestroyOnSceneEnd>
{
public:
  static void DoTick(float dt)
  {
    std::vector<std::shared_ptr<Entity>> toDelete;
    for(auto tuple : Tuples)
    {
      DestroyOnSceneEnd* actor = std::get<DestroyOnSceneEnd*>(tuple.second);
      toDelete.push_back(actor->Entity());
    }
    
    for(auto entity : toDelete)
      GameManager::Get().DestroyEntity(entity);
  }
};
