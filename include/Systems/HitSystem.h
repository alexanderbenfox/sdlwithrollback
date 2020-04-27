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

      // reset the merge context
      hurtboxController->mergeContext.hitThisFrame = false;
      hurtboxController->mergeContext.hitOnLeftSide = false;

      for (auto hitbox : ComponentManager<Hitbox>::Get().All())
      {
        // if the hitbox has hit something (will change this to checking if it has hit the entity?)
        if(hitbox->hit)
          continue;

        if (!hitbox->ShareOwner(hurtbox) && hitbox->rect.Collides(hurtbox->rect))
        {
          hitbox->hit = true;
          hitbox->hitting = true;
          // change the state variable that will be evaluated on the processing of inputs. probably a better way to do this...
          hurtboxController->mergeContext.hitThisFrame = true;

          // calculate the strike vector
          hurtboxController->mergeContext.hitOnLeftSide =  hitbox->strikeVector.x > 0;

          hurtboxController->mergeContext.frameData = hitbox->frameData;

          // this needs to be made better
          if(hitbox->strikeVector.x < 0)
            hurtboxController->mergeContext.frameData.knockback.x = -hitbox->frameData.knockback.x;

          GameManager::Get().ActivateHitStop(hitbox->frameData.hitstop);
        }
      }
    }
  }
};

class StrikeVectorSystem : public ISystem<Hitbox, Hurtbox>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      Hitbox* hitbox = std::get<Hitbox*>(tuple.second);
      Hurtbox* hurtbox = std::get<Hurtbox*>(tuple.second);

      int strikeDir = hitbox->rect.GetCenter().x > hurtbox->rect.GetCenter().x ? 1 : -1;
      hitbox->strikeVector = Vector2<int>(strikeDir, 0);
    }
  }
};

// don't know what else to do for this so this is probably stupid
class SendHittingStateSystem : public ISystem<Hitbox, GameActor>
{
public:
  static void DoTick(float dt)
  {
    // only update this on active frames
    if (dt == 0)
      return;

    for (auto tuple : Tuples)
    {
      Hitbox* hitbox = std::get<Hitbox*>(tuple.second);
      GameActor* hitboxController = std::get<GameActor*>(tuple.second);

      hitboxController->mergeContext.hitting = hitbox->hitting;
      hitbox->hitting = false;
    }
  }
};

class FrameAdvantageSystem : public ISystem<AttackStateComponent, RenderProperties>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      AttackStateComponent* atkState = std::get<AttackStateComponent*>(tuple.second);
      RenderProperties* properties = std::get<RenderProperties*>(tuple.second);

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
        properties->SetDisplayColor(0, 0, 255);
      else if (attackerFrameAdvantage < 0)
        properties->SetDisplayColor(255, 0, 0);
      else
        properties->SetDisplayColor(255, 255, 255);

    }
  }
};
