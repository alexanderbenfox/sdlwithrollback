#pragma once
#include "Systems/ISystem.h"

#include "Systems/WallPush/WallPushComponent.h"
#include "Components/Hitbox.h"
#include "Components/Hurtbox.h"
#include "Components/StateComponent.h"
#include "Components/StateComponents/HitStateComponent.h"
#include "Components/SFXComponent.h"

class HitSystem : public IMultiSystem<SysComponents<Hurtbox, StateComponent, TeamComponent, SFXComponent>, SysComponents<Hitbox, Hurtbox, StateComponent, TeamComponent, Rigidbody>>
{
public:
  static void DoTick(float dt)
  {
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
      SFXComponent* sfx = std::get<SFXComponent*>(tuple.second);
      
      for(auto subTuple : SubSystem::Tuples)
      {
        Hitbox* hitbox = std::get<Hitbox*>(subTuple.second);
        StateComponent* hitboxController = std::get<StateComponent*>(subTuple.second);
        Hurtbox* hitterHurtbox = std::get<Hurtbox*>(subTuple.second);
        TeamComponent* hitterTeam = std::get<TeamComponent*>(subTuple.second);
        Rigidbody* hitterRb = std::get<Rigidbody*>(subTuple.second);

        if (hurtboxTeam->team == hitterTeam->team)
          continue;

        hitboxController->hitting = false;

        // if the hitbox has hit something (will change this to checking if it has hit the entity?)
        if (hitbox->hitFlag)
          continue;

        if (hitbox->rect.Intersects(hurtbox->rect))
        {
          // do hitbox stuff first
          hitbox->hitFlag = true;
          hitboxController->hitting = true;
          int strikeDir = hitbox->rect.GetCenter().x > hitterHurtbox->rect.GetCenter().x ? 1 : -1;

          // change the state variable that will be evaluated on the processing of inputs. probably a better way to do this...
          hurtboxController->hitThisFrame = true;
          hurtboxController->hitData = hitbox->tData;
          Vector2<float> knockback = hitbox->tData.knockback;

          // this needs to be made better
          if (strikeDir < 0)
          {
            knockback.x *= -1.0f;
            hurtboxController->hitData.knockback.x = knockback.x;
          }

          // apply hitter knockback if in the corner here
          if ((hurtboxController->onLeftSide && HasState(hurtboxController->collision, CollisionSide::LEFT)) ||
            (!hurtboxController->onLeftSide && HasState(hurtboxController->collision, CollisionSide::RIGHT)))
          {
            const float maxCornerKnockback = 100.0f;

            hitbox->Owner()->AddComponent<WallPushComponent>();
            auto push = hitbox->Owner()->GetComponent<WallPushComponent>();
            float pushBackAmount = knockback.x / 4.0f;
            push->pushAmount = -std::min(pushBackAmount, maxCornerKnockback);
            push->velocity = -knockback.x / 2.0f;
            push->amountPushed = 0.0f;
          }

          sfx->showLocation = (Vector2<float>)hitbox->rect.GetIntersection(hurtbox->rect).GetCenter();

          //! this will trigger self-destruction if this entity is intended to be destroyed on hit
          hitbox->OnCollision(hurtbox);
        }
      }
    }
  }
};

class ThrowSystem : public IMultiSystem<SysComponents<Throwbox, TeamComponent, StateComponent>, SysComponents<Hurtbox, TeamComponent, StateComponent>>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : MainSystem::Tuples)
    {
      Throwbox* throwbox = std::get<Throwbox*>(tuple.second);

      if (throwbox->hitFlag)
        continue;

      TeamComponent* grapplerTeam = std::get<TeamComponent*>(tuple.second);
      StateComponent* grapplerController = std::get<StateComponent*>(tuple.second);
      grapplerController->triedToThrowThisFrame = true;

      for (auto subTuple : SubSystem::Tuples)
      {
        Hurtbox* grappledHurtbox = std::get<Hurtbox*>(subTuple.second);
        TeamComponent* grappledTeam = std::get<TeamComponent*>(subTuple.second);
        StateComponent* grappledController = std::get<StateComponent*>(subTuple.second);

        if (grappledController->stanceState == StanceState::KNOCKDOWN || grapplerTeam->team == grappledTeam->team)
          continue;

        if (throwbox->rect.Intersects(grappledHurtbox->rect))
        {
          throwbox->hitFlag = true;
          grapplerController->throwSuccess = true;

          grappledController->thrownThisFrame = true;
          grappledController->hitData = throwbox->tData;

          // this needs to be made better
          if (grappledController->onLeftSide)
            grappledController->hitData.knockback.x = -throwbox->tData.knockback.x;

          grappledController->Owner()->AddComponent<ReceivedGrappleAction>();
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
