#pragma once
#include "Systems/ISystem.h"
#include "Components/Hitbox.h"
#include "Components/GameActor.h"

//!
//const float secPerFrame = 1.0f / animation_fps;

class HitSystem : public ISystem<Hurtbox, GameActor>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      Hurtbox* hurtbox = std::get<Hurtbox*>(tuple.second);
      GameActor* hurtboxController = std::get<GameActor*>(tuple.second);

      for (auto hitbox : ComponentManager<Hitbox>::Get().All())
      {
        if(hitbox->hit)
          continue;
        if (!hitbox->ShareOwner(hurtbox) && hitbox->rect.Collides(hurtbox->rect))
        {
          bool ok;
          if(hurtboxController->GetAction()->GetAction() == ActionState::HITSTUN)
            ok = true;
          hitbox->hit = true;
          // change the state variable that will be evaluated on the processing of inputs. probably a better way to do this...
          hurtboxController->mergeContext.hitThisFrame = true;
          double amtLeft = hurtbox->rect.GetCenter().x - hitbox->rect.Beg().x;
          double amtRight = hitbox->rect.End().x - hurtbox->rect.GetCenter().x;
          hurtboxController->mergeContext.hitOnLeftSide =  amtLeft > amtRight;
          hurtboxController->mergeContext.frameData = hitbox->frameData;
          if(amtLeft < amtRight)
            hurtboxController->mergeContext.frameData.knockback.x = -hitbox->frameData.knockback.x;
        }
      }
    }
  }
};

class FrameAdvantageSystem : public ISystem<AttackStateComponent, AnimationRenderer>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      AttackStateComponent* atkState = std::get<AttackStateComponent*>(tuple.second);
      AnimationRenderer* renderer = std::get<AnimationRenderer*>(tuple.second);

      // because the animation system will end up incrementing and ending the attack
      // state component, a shitty work around is that the frame advantage system
      // will end the color change a frame early
      // i will have to fix the way that the attack animations work first
      /*if (atkState->GetRemainingFrames() <= 1)
      {
        renderer->SetDisplayColor(255, 255, 255);
        continue;
      }*/


      // initially disadvantage if nothing is currently blocking or hit by it
      int attackerFrameAdvantage = -atkState->GetRemainingFrames();

      for (auto hitEntity : ComponentManager<HitStateComponent>::Get().All())
      {
        attackerFrameAdvantage += hitEntity->GetRemainingFrames();
      }

      for (auto otherAttackingEntity : ComponentManager<AttackStateComponent>::Get().All())
      {
        if(otherAttackingEntity.get() != atkState)
          attackerFrameAdvantage += otherAttackingEntity->GetRemainingFrames();
      }

      if (attackerFrameAdvantage > 0)
        renderer->SetDisplayColor(0, 0, 255);
      else if (attackerFrameAdvantage < 0)
        renderer->SetDisplayColor(255, 0, 0);
      else
        renderer->SetDisplayColor(255, 255, 255);

    }
  }
};
