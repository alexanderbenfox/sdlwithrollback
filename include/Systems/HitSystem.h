#pragma once
#include "Systems/ISystem.h"
#include "Components/Hitbox.h"
#include "Components/Hurtbox.h"
#include "Components/StateComponent.h"
#include "Components/StateComponents/HitStateComponent.h"

class HitSystem : public IMultiSystem<SysComponents<Hurtbox, StateComponent, TeamComponent>, SysComponents<Hitbox, Hurtbox, StateComponent, TeamComponent>>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : MainSystem::Tuples)
    {
      StateComponent* hurtboxController = std::get<StateComponent*>(tuple.second);

      // reset the merge context
      hurtboxController->hitThisFrame = false;
    }

    if(dt <= 0)
      return;

    // reset hitting state only on timed update
    for(auto tuple : MainSystem::Tuples)
    {
      StateComponent* hurtboxController = std::get<StateComponent*>(tuple.second);
      hurtboxController->hitting = false;
    }

    for (auto tuple : MainSystem::Tuples)
    {
      StateComponent* hurtboxController = std::get<StateComponent*>(tuple.second);
      Hurtbox* hurtbox = std::get<Hurtbox*>(tuple.second);
      TeamComponent* hurtboxTeam = std::get<TeamComponent*>(tuple.second);
      
      for(auto subTuple : SubSystem::Tuples)
      {
        Hitbox* hitbox = std::get<Hitbox*>(subTuple.second);
        StateComponent* hitboxController = std::get<StateComponent*>(subTuple.second);
        Hurtbox* hitterHurtbox = std::get<Hurtbox*>(subTuple.second);
        TeamComponent* hitterTeam = std::get<TeamComponent*>(subTuple.second);

        if (hurtboxTeam->team == hitterTeam->team)
          continue;

        hitboxController->hitting = false;

        // if the hitbox has hit something (will change this to checking if it has hit the entity?)
        if (hitbox->hit)
          continue;

        if (hitbox->rect.Intersects(hurtbox->rect))
        {
          // do hitbox stuff first
          hitbox->hit = true;
          hitboxController->hitting = true;
          int strikeDir = hitbox->rect.GetCenter().x > hitterHurtbox->rect.GetCenter().x ? 1 : -1;

          // change the state variable that will be evaluated on the processing of inputs. probably a better way to do this...
          hurtboxController->hitThisFrame = true;
          hurtboxController->hitData = hitbox->hitData;

          // this needs to be made better
          if (strikeDir < 0)
            hurtboxController->hitData.knockback.x = -hitbox->hitData.knockback.x;

          GameManager::Get().ActivateHitStop(10);

          //! this will trigger self-destruction if this entity is intended to be destroyed on hit
          hitbox->OnCollision(hurtbox);
        }
      }
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
