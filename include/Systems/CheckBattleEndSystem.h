#pragma once
#include "Systems/ISystem.h"
#include "Components/StateComponent.h"
#include "Components/GameActor.h"

class CheckBattleEndSystem : public ISystem<StateComponent, GameActor>
{
public:
  static void DoTick(float dt)
  {
    bool readyToTransition = false;
    for(auto& tuple : Tuples)
    {
      // just including game actor so this only affects controllable units
      GameActor* actor = std::get<GameActor*>(tuple.second);
      StateComponent* state = std::get<StateComponent*>(tuple.second);
      
      if(state->hp <= 0)
      {
        state->MarkLoser();
      }

      if(state->actionState != ActionState::HITSTUN && state->hp <= 0)
      {
        readyToTransition = true;
      }
    }

    if(readyToTransition)
      GameManager::Get().RequestSceneChange(SceneType::POSTMATCH);
  }
};
