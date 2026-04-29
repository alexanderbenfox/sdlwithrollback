#pragma once
#include "Core/ECS/ISystem.h"
#include "Managers/GameManagement.h"

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
    PROFILE_FUNCTION();
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
          // For melee attacks, direction is hitbox relative to attacker's body.
          // For projectiles, the hitbox IS the entity, so use hitbox vs defender instead.
          int strikeDir = hitbox.destroyOnHit
            ? (hurtbox.rect.GetCenter().x > hitbox.rect.GetCenter().x ? 1 : -1)
            : (hitbox.rect.GetCenter().x > hitterHurtbox.rect.GetCenter().x ? 1 : -1);

          // change the state variable that will be evaluated on the processing of inputs. probably a better way to do this...
          hurtboxController.hitThisFrame = true;
          hurtboxController.hitData = hitbox.tData;
          Vector2<float> knockback = hitbox.tData.knockback;

          // if its a jumping attack, set attack hit type to overhead
          if (hitboxController.stanceState == StanceState::JUMPING)
            hurtboxController.hitData.type = HitType::High;

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

          // destroy projectile-type entities on hit
          if (hitbox.destroyOnHit)
            GameManager::Get().TriggerEndOfFrame([e2]() { GameManager::Get().DestroyEntity(e2); });
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
    PROFILE_FUNCTION();
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
