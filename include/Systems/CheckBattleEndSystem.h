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
      StateComponent* state = std::get<StateComponent*>(tuple.second);
      GameActor* actor = std::get<GameActor*>(tuple.second);
      if(state->hp <= 0)
      {
        state->MarkLoser();
      }

      if(actor->GetActionState() != ActionState::HITSTUN && state->hp <= 0)
      {
        readyToTransition = true;
      }
    }

    if(readyToTransition)
      GameManager::Get().RequestSceneChange(SceneType::POSTMATCH);
  }
};
