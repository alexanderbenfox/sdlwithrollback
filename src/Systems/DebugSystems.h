#pragma once
#include "Core/ECS/ISystem.h"

#include "Components/Animator.h"
#include "Components/RenderComponent.h"
#include "Components/StateComponents/AttackStateComponent.h"
#include "Components/StateComponents/HitStateComponent.h"
#include "Components/FighterFSMComponent.h"

#include "Managers/AnimationCollectionManager.h"

// Debug-only system: tints attacker sprite to visualize frame advantage.
// Blue = positive (attacker recovers first), Red = negative, White = neutral.
class DebugFrameAdvantageSystem : public IMultiSystem<SysComponents<AttackStateComponent, Animator, RenderProperties>, SysComponents<HitStateComponent, FighterFSMComponent>>
{
public:
  static void DoTick(float dt)
  {
    PROFILE_FUNCTION();
    for (const EntityID& entity : MainSystem::Registered)
    {
      Animator& animator = ComponentArray<Animator>::Get().GetComponent(entity);
      RenderProperties& properties = ComponentArray<RenderProperties>::Get().GetComponent(entity);

      // initially disadvantage if nothing is currently blocking or hit by it
      int attackerAnimTotalFrames = GAnimArchive.GetAnimationData(animator.animCollectionID, animator.currentAnimationName)->GetFrameCount() - 1;
      // set advantage to current remaining frames
      int attackerFrameAdvantage = -(attackerAnimTotalFrames - animator.frame);

      for (const EntityID& e2 : SubSystem::Registered)
      {
        const FighterFSMComponent& fsm = ComponentArray<FighterFSMComponent>::Get().GetComponent(e2);
        int remainingFrames = fsm.stateTotalFrames - fsm.stateFrame;

        attackerFrameAdvantage += remainingFrames;
      }

      if (attackerFrameAdvantage > 0)
        properties.SetDisplayColor(0, 0, 255);
      else if (attackerFrameAdvantage < 0)
        properties.SetDisplayColor(255, 0, 0);
      else
        properties.SetDisplayColor(255, 255, 255);
    }
  }
};
