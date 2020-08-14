#pragma once
#include "Systems/ISystem.h"
#include "Components/StateComponent.h"
#include "Components/Actors/GameActor.h"

class CheckBattleEndSystem : public ISystem<StateComponent, GameActor>
{
public:
  static void DoTick(float dt)
  {
    bool readyToTransition = false;
    for(const EntityID& entity : Registered)
    {
      // just including game actor so this only affects controllable units
      GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);
      StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);
      
      if(state.hp <= 0)
      {
        GameManager::Get().GetEntityByID(entity)->AddComponent<LoserComponent>();
      }

      if(state.actionState != ActionState::HITSTUN && state.hp <= 0)
      {
        readyToTransition = true;
      }
    }

    if(readyToTransition)
      GameManager::Get().RequestSceneChange(SceneType::POSTMATCH);
  }
};
