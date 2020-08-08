#pragma once
#include "Systems/ISystem.h"

#include "Components/ActionComponents.h"
#include "Components/Animator.h"
#include "Components/GameActor.h"

struct AnimationListenerSystem : public ISystem<WaitForAnimationComplete, Animator, GameActor>
{
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      Animator* animator = std::get<Animator*>(tuple.second);
      GameActor* actor = std::get<GameActor*>(tuple.second);
      // check for end of animation
      if (!animator->looping && animator->frame == (animator->GetCurrentAnimation().GetFrameCount() - 1))
      {
        // can now look for another input to change state
        actor->actionTimerComplete = true;
        // force check for new state here
        actor->forceNewInputOnNextFrame = true;
      }
    }
  }
};
