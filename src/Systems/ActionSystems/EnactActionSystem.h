#pragma once
#include "Core/ECS/ISystem.h"
#include "Core/Utility/DeferGuard.h"

#include "Components/ActionComponents.h"
#include "Components/Animator.h"
#include "Components/RenderComponent.h"
#include "Components/Hurtbox.h"
#include "Components/Actors/GameActor.h"

#include "Managers/AnimationCollectionManager.h"

struct EnactAnimationActionSystem : public ISystem<EnactActionComponent, AnimatedActionComponent, Animator, RenderProperties, RenderComponent<RenderType>>
{
  static void DoTick(float dt)
  {
    if (dt <= 0)
      return;

    for (const EntityID& entity : Registered)
    {
      AnimatedActionComponent& action = ComponentArray<AnimatedActionComponent>::Get().GetComponent(entity);
      Animator& animator = ComponentArray<Animator>::Get().GetComponent(entity);
      RenderProperties& properties = ComponentArray<RenderProperties>::Get().GetComponent(entity);
      RenderComponent<RenderType>& renderer = ComponentArray<RenderComponent<RenderType>>::Get().GetComponent(entity);

      Animation* actionAnimation = animator.Play(action.animation, action.isLoopedAnimation, action.playSpeed, action.forceAnimRestart);
      properties.horizontalFlip = !action.isFacingRight;
      properties.anchor = actionAnimation->GetMainAnchor().first;
      properties.offset = actionAnimation->GetMainAnchor().second;
      properties.renderScaling = actionAnimation->GetRenderScaling();
      if (actionAnimation)
      {
        // render from the sheet of the new animation
        renderer.SetRenderResource(actionAnimation->GetSheetTexture<RenderType>());
        renderer.sourceRect = actionAnimation->GetFrameSrcRect(0);
      }
    }
  }
};

#include "Components/StateComponents/AttackStateComponent.h"

struct EnactAttackActionSystem : public ISystem<EnactActionComponent, AnimatedActionComponent, AttackActionComponent, Animator>
{
  static void DoTick(float dt)
  {
    if (dt <= 0)
      return;

    for (const EntityID& entity : Registered)
    {
      AnimatedActionComponent& actionData = ComponentArray<AnimatedActionComponent>::Get().GetComponent(entity);
      AttackActionComponent& action = ComponentArray<AttackActionComponent>::Get().GetComponent(entity);
      Animator& animator = ComponentArray<Animator>::Get().GetComponent(entity);

      if (GAnimArchive.GetCollection(animator.animCollectionID).GetEventList(actionData.animation))
      {
        GameManager::Get().GetEntityByID(entity)->AddComponent<AttackStateComponent>();
        GameManager::Get().GetEntityByID(entity)->GetComponent<AttackStateComponent>()->attackAnimation = actionData.animation;
      }
    }
  }
};

struct EnactGrappleActionSystem : public ISystem<EnactActionComponent, GrappleActionComponent, Rigidbody>
{
  static void DoTick(float dt)
  {
    if (dt <= 0)
      return;

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
      if (dt <= 0)
        return;

      MovingActionComponent& action = ComponentArray<MovingActionComponent>::Get().GetComponent(entity);
      Rigidbody& rb = ComponentArray<Rigidbody>::Get().GetComponent(entity);

      if (action.horizontalMovementOnly)
        rb.velocity.x = action.velocity.x;
      else
        rb.velocity = action.velocity;
    }
  }
};

#include "Components/StateComponent.h"
#include "Components/SFXComponent.h"

struct EnactActionDamageSystem : public ISystem<EnactActionComponent, ReceivedDamageAction, StateComponent, SFXComponent>
{
  static void DoTick(float dt)
  {
    if (dt <= 0)
      return;

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
    if (dt <= 0)
      return;

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
    DeferGuard guard;
    for (const EntityID& entity : Registered)
    {
      EntityID id = entity;
      RunOnDeferGuardDestroy(entity,
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
