#pragma once
#include "Core/ECS/ISystem.h"

// for wall push
#include "Components/ActionComponents.h"

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

    for (const EntityID& e1 : MainSystem::Registered)
    {
      StateComponent& hurtboxController = ComponentArray<StateComponent>::Get().GetComponent(e1);
      Hurtbox& hurtbox = ComponentArray<Hurtbox>::Get().GetComponent(e1);
      TeamComponent& hurtboxTeam = ComponentArray<TeamComponent>::Get().GetComponent(e1);
      SFXComponent& sfx = ComponentArray<SFXComponent>::Get().GetComponent(e1);
      
      for(const EntityID& e2 : SubSystem::Registered)
      {
        Hitbox& hitbox = ComponentArray<Hitbox>::Get().GetComponent(e2);
        StateComponent& hitboxController = ComponentArray<StateComponent>::Get().GetComponent(e2);
        Hurtbox& hitterHurtbox = ComponentArray<Hurtbox>::Get().GetComponent(e2);
        TeamComponent& hitterTeam = ComponentArray<TeamComponent>::Get().GetComponent(e2);
        Rigidbody& hitterRb = ComponentArray<Rigidbody>::Get().GetComponent(e2);

        if (hurtboxTeam.team == hitterTeam.team)
          continue;

        hitboxController.hitting = false;

        // if the hitbox has hit something (will change this to checking if it has hit the entity?)
        if (hitbox.hitFlag)
          continue;

        if (hitbox.rect.Intersects(hurtbox.rect))
        {
          // do hitbox stuff first
          hitbox.hitFlag = true;
          hitboxController.hitting = true;
          int strikeDir = hitbox.rect.GetCenter().x > hitterHurtbox.rect.GetCenter().x ? 1 : -1;

          // change the state variable that will be evaluated on the processing of inputs. probably a better way to do this...
          hurtboxController.hitThisFrame = true;
          hurtboxController.hitData = hitbox.tData;
          Vector2<float> knockback = hitbox.tData.knockback;

          // this needs to be made better
          if (strikeDir < 0)
          {
            knockback.x *= -1.0f;
            hurtboxController.hitData.knockback.x = knockback.x;
          }

          // apply hitter knockback if in the corner here
          if ((hurtboxController.onLeftSide && HasState(hurtboxController.collision, CollisionSide::LEFT)) ||
            (!hurtboxController.onLeftSide && HasState(hurtboxController.collision, CollisionSide::RIGHT)))
          {
            const float maxCornerKnockback = 100.0f;

            GameManager::Get().GetEntityByID(e2)->AddComponent<WallPushComponent>();
            auto push = GameManager::Get().GetEntityByID(e2)->GetComponent<WallPushComponent>();
            float pushBackAmount = knockback.x / 4.0f;
            push->pushAmount = -std::min(pushBackAmount, maxCornerKnockback);
            push->velocity = -knockback.x / 2.0f;
            push->amountPushed = 0.0f;
          }

          sfx.showLocation = (Vector2<float>)hitbox.rect.GetIntersection(hurtbox.rect).GetCenter();

          //! this will trigger self-destruction if this entity is intended to be destroyed on hit
          hitbox.OnCollision(e2, &hurtbox);
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
    for (const EntityID& entity : MainSystem::Registered)
    {
      Throwbox& throwbox = ComponentArray<Throwbox>::Get().GetComponent(entity);

      if (throwbox.hitFlag)
        continue;

      TeamComponent& grapplerTeam = ComponentArray<TeamComponent>::Get().GetComponent(entity);
      StateComponent& grapplerController = ComponentArray<StateComponent>::Get().GetComponent(entity);
      grapplerController.triedToThrowThisFrame = true;

      for (const EntityID& e2 : SubSystem::Registered)
      {
        Hurtbox& grappledHurtbox = ComponentArray<Hurtbox>::Get().GetComponent(e2);
        TeamComponent& grappledTeam = ComponentArray<TeamComponent>::Get().GetComponent(e2);
        StateComponent& grappledController = ComponentArray<StateComponent>::Get().GetComponent(e2);

        if (grappledController.stanceState == StanceState::KNOCKDOWN || grapplerTeam.team == grappledTeam.team)
          continue;

        if (throwbox.rect.Intersects(grappledHurtbox.rect))
        {
          throwbox.hitFlag = true;
          grapplerController.throwSuccess = true;

          grappledController.thrownThisFrame = true;
          grappledController.hitData = throwbox.tData;

          // this needs to be made better
          if (grappledController.onLeftSide)
            grappledController.hitData.knockback.x = -throwbox.tData.knockback.x;

          GameManager::Get().GetEntityByID(e2)->AddComponent<ReceivedGrappleAction>();
        }
      }
    }
  }
};

#include "AssetManagement/AnimationCollectionManager.h"

class FrameAdvantageSystem : public ISystem<AttackStateComponent, Animator, RenderProperties>
{
public:
  static void DoTick(float dt)
  {
    for (const EntityID& entity : Registered)
    {
      AttackStateComponent& atkState = ComponentArray<AttackStateComponent>::Get().GetComponent(entity);
      Animator& animator = ComponentArray<Animator>::Get().GetComponent(entity);
      RenderProperties& properties = ComponentArray<RenderProperties>::Get().GetComponent(entity);

      // initially disadvantage if nothing is currently blocking or hit by it
      int attackerAnimTotalFrames = GAnimArchive.GetAnimationData(animator.animCollectionID, animator.currentAnimationName)->GetFrameCount() - 1;
      // set advantage to current remaining frames
      int attackerFrameAdvantage = -(attackerAnimTotalFrames - animator.frame);

      ComponentArray<HitStateComponent>::Get().ForEach([&attackerFrameAdvantage](HitStateComponent& hitEntity)
      {
        attackerFrameAdvantage += hitEntity.GetRemainingFrames();
      });

      // this is for stuff like fireball
      /*ComponentArray<AttackStateComponent>::Get().ForEach([&attackerFrameAdvantage, &atkState](AttackStateComponent& otherAttackingEntity)
      {
          if (&otherAttackingEntity != &atkState)
          {
            attackerFrameAdvantage += otherAttackingEntity.GetRemainingFrames();
          }
      });*/

      if (attackerFrameAdvantage > 0)
        properties.SetDisplayColor(0, 0, 255);
      else if (attackerFrameAdvantage < 0)
        properties.SetDisplayColor(255, 0, 0);
      else
        properties.SetDisplayColor(255, 255, 255);

    }
  }
};
