#include "EnactActionSystem.h"
#include "Managers/GameManagement.h"
#include "Managers/AnimationCollectionManager.h"

#include "Core/Utility/DeferGuard.h"

void EnactAnimationActionSystem::DoTick(float dt)
{
  if (dt <= 0)
    return;

  for (const EntityID& entity : Registered)
  {
    AnimatedActionComponent& action = ComponentArray<AnimatedActionComponent>::Get().GetComponent(entity);
    PlayAnimation(entity, action.animation, action.isLoopedAnimation, action.playSpeed, action.forceAnimRestart, action.isFacingRight);
  }
}

void EnactAnimationActionSystem::PlayAnimation(EntityID entity, const std::string& animation, bool looped, float playSpeed, bool forceAnimRestart, bool facingRight)
{
  Animator& animator = ComponentArray<Animator>::Get().GetComponent(entity);
  RenderProperties& properties = ComponentArray<RenderProperties>::Get().GetComponent(entity);
  RenderComponent<RenderType>& renderer = ComponentArray<RenderComponent<RenderType>>::Get().GetComponent(entity);

  Animation* actionAnimation = animator.Play(animation, looped, playSpeed, forceAnimRestart);
  properties.horizontalFlip = !facingRight;
  properties.anchor = actionAnimation->GetAnchorForAnimFrame(0).first;
  properties.offset = actionAnimation->GetAnchorForAnimFrame(0).second;
  properties.renderScaling = actionAnimation->GetRenderScaling();

  renderer.SetRenderResource(actionAnimation->GetSheetTexture<RenderType>());
  renderer.sourceRect = actionAnimation->GetFrameSrcRect(0);
}


void EnactAttackActionSystem::DoTick(float dt)
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

void EnactGrappleActionSystem::DoTick(float dt)
{
  if (dt <= 0)
    return;

  for (const EntityID& entity : Registered)
  {
    Rigidbody& rb = ComponentArray<Rigidbody>::Get().GetComponent(entity);
    rb.ignoreDynamicColliders = true;
  }
}

void EnactActionMovementSystem::DoTick(float dt)
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

void EnactActionDamageSystem::DoTick(float dt)
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

void EnactGrappledSystem::DoTick(float dt)
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

void CleanUpActionSystem::PostUpdate()
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
