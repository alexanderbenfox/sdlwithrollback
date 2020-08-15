#pragma once
#include "Core/ECS/ISystem.h"

#include "Components/ActionComponents.h"
#include "Components/Animator.h"
#include "Components/RenderComponent.h"
#include "Components/Hurtbox.h"
#include "Components/Actors/GameActor.h"

struct EnactAnimationActionSystem : public ISystem<EnactActionComponent, AnimatedActionComponent, Animator, RenderProperties, RenderComponent<RenderType>, Hurtbox, GameActor>
{
  static void DoTick(float dt)
  {
    for (const EntityID& entity : Registered)
    {
      AnimatedActionComponent& action = ComponentArray<AnimatedActionComponent>::Get().GetComponent(entity);
      Animator& animator = ComponentArray<Animator>::Get().GetComponent(entity);
      RenderProperties& properties = ComponentArray<RenderProperties>::Get().GetComponent(entity);
      RenderComponent<RenderType>& renderer = ComponentArray<RenderComponent<RenderType>>::Get().GetComponent(entity);
      Hurtbox& hurtbox = ComponentArray<Hurtbox>::Get().GetComponent(entity);
      GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);

      if (animator.AnimationLib() && animator.AnimationLib()->GetAnimation(action.animation))
      {
        Animation* actionAnimation = animator.Play(action.animation, action.isLoopedAnimation, action.playSpeed, action.forceAnimRestart);
        properties.horizontalFlip = !action.isFacingRight;
        properties.offset = -animator.AnimationLib()->GetRenderOffset(action.animation, !action.isFacingRight, (int)std::floor(hurtbox.unscaledRect.Width()));
        if (actionAnimation)
        {
          // render from the sheet of the new animation
          renderer.SetRenderResource(actionAnimation->GetSheetTexture<RenderType>());
          renderer.sourceRect = actionAnimation->GetFrameSrcRect(0);
        }
      }
    }
  }
};

#include "Components/StateComponents/AttackStateComponent.h"

struct EnactAttackActionSystem : public ISystem<EnactActionComponent, AnimatedActionComponent, AttackActionComponent, Animator>
{
  static void DoTick(float dt)
  {
    for (const EntityID& entity : Registered)
    {
      AnimatedActionComponent& actionData = ComponentArray<AnimatedActionComponent>::Get().GetComponent(entity);
      AttackActionComponent& action = ComponentArray<AttackActionComponent>::Get().GetComponent(entity);
      Animator& animator = ComponentArray<Animator>::Get().GetComponent(entity);

      if (animator.AnimationLib()->GetAnimation(actionData.animation) && animator.AnimationLib()->GetEventList(actionData.animation))
      {
        GameManager::Get().GetEntityByID(entity)->AddComponent<AttackStateComponent>();
        GameManager::Get().GetEntityByID(entity)->GetComponent<AttackStateComponent>()->Init(animator.AnimationLib()->GetAnimation(actionData.animation), animator.AnimationLib()->GetEventList(actionData.animation));
      }
    }
  }
};

struct EnactGrappleActionSystem : public ISystem<EnactActionComponent, GrappleActionComponent, Rigidbody>
{
  static void DoTick(float dt)
  {
    for (const EntityID& entity : Registered)
    {
      Rigidbody& rb = ComponentArray<Rigidbody>::Get().GetComponent(entity);
      rb.ignoreDynamicColliders = true;
    }
  }
};

#include "Components/Rigidbody.h"

struct EnactActionMovementSystem : public ISystem<EnactActionComponent, MovingActionComponent, Rigidbody>
{
  static void DoTick(float dt)
  {
    for (const EntityID& entity : Registered)
    {
      MovingActionComponent& action = ComponentArray<MovingActionComponent>::Get().GetComponent(entity);
      Rigidbody& rb = ComponentArray<Rigidbody>::Get().GetComponent(entity);

      if (action.horizontalMovementOnly)
        rb._vel.x = action.velocity.x;
      else
        rb._vel = action.velocity;
    }
  }
};

#include "Components/StateComponent.h"
#include "Components/SFXComponent.h"

struct EnactActionDamageSystem : public ISystem<EnactActionComponent, ReceivedDamageAction, StateComponent, SFXComponent>
{
  static void DoTick(float dt)
  {
    for (const EntityID& entity : Registered)
    {
      ReceivedDamageAction& action = ComponentArray<ReceivedDamageAction>::Get().GetComponent(entity);
      StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);

      if (!state.invulnerable)
      {
        state.hp -= action.damageAmount;
      }

      SFXComponent& sfx = ComponentArray<SFXComponent>::Get().GetComponent(entity);
      if (GlobalVars::ShowHitEffects)
      {
        if (action.isBlocking)
          sfx.ShowBlockSparks(state.onLeftSide);
        else if (!action.fromGrapple)
          sfx.ShowHitSparks(state.onLeftSide);
      }

      if (!action.fromGrapple)
      {
        GameManager::Get().ActivateHitStop(action.isBlocking ? GlobalVars::HitStopFramesOnBlock : GlobalVars::HitStopFramesOnHit);
      }
    }
  }
};

struct EnactGrappledSystem : public ISystem<EnactActionComponent, ReceivedGrappleAction, StateComponent, Rigidbody>
{
  static void DoTick(float dt)
  {
    for (const EntityID& entity : Registered)
    {
      ReceivedGrappleAction& action = ComponentArray<ReceivedGrappleAction>::Get().GetComponent(entity);
      StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);
      Rigidbody& rb = ComponentArray<Rigidbody>::Get().GetComponent(entity);

      state.thrownThisFrame = false;
      rb.ignoreDynamicColliders = true;

      GameManager::Get().GetEntityByID(entity)->AddComponent<TimedActionComponent>();
      GameManager::Get().GetEntityByID(entity)->GetComponent<TimedActionComponent>()->totalFrames = action.damageAndKnockbackDelay;
    }
  }
};

struct CleanUpActionSystem : public ISystem<EnactActionComponent>
{
  static void PostUpdate()
  {
    DeferScopeGuard guard;
    for (const EntityID& entity : Registered)
    {
      defer(entity,
        GameManager::Get().GetEntityByID(entity)->RemoveComponent<EnactActionComponent>();
        // we want to be listening for a new action now
        GameManager::Get().GetEntityByID(entity)->AddComponent<InputListenerComponent>();
      );
    }
  }
};

struct EnactAggregate
{
  static void Check(Entity* entity)
  {
    EnactAnimationActionSystem::Check(entity);
    EnactAttackActionSystem::Check(entity);
    EnactGrappleActionSystem::Check(entity);
    EnactActionMovementSystem::Check(entity);
    EnactActionDamageSystem::Check(entity);
    EnactGrappledSystem::Check(entity);
    CleanUpActionSystem::Check(entity);

    // just needs to be here to generate component id for losercomponent
    //LoserSystem::Check(entity);
  }

  static void DoTick(float dt)
  {
    EnactAnimationActionSystem::DoTick(dt);
    EnactAttackActionSystem::DoTick(dt);
    EnactGrappleActionSystem::DoTick(dt);
    EnactActionMovementSystem::DoTick(dt);
    EnactActionDamageSystem::DoTick(dt);
    EnactGrappledSystem::DoTick(dt);
  }
};