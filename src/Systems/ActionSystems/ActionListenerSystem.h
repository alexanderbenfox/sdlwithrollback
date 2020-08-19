#pragma once
#include "Core/ECS/ISystem.h"

#include "Components/ActionComponents.h"
#include "Components/Animator.h"
#include "Components/Actors/GameActor.h"

#include "AssetManagement/AnimationCollectionManager.h"

struct AnimationListenerSystem : public ISystem<WaitForAnimationComplete, Animator, GameActor>
{
  static void DoTick(float dt)
  {
    for (const EntityID& entity : Registered)
    {
      Animator& animator = ComponentArray<Animator>::Get().GetComponent(entity);
      GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);

      // check for end of animation
      if (!animator.looping && animator.frame == (GAnimArchive.GetAnimationData(animator.animCollectionID, animator.currentAnimationName)->GetFrameCount() - 1))
      {
        // can now look for another input to change state
        actor.actionTimerComplete = true;
        // force check for new state here
        actor.forceNewInputOnNextFrame = true;
      }
    }
  }
};
