#pragma once
#include "Core/ECS/ISystem.h"

#include "Components/ActionComponents.h"
#include "Components/Animator.h"
#include "Components/RenderComponent.h"
#include "Components/Hurtbox.h"
#include "Components/Actors/GameActor.h"
#include "Components/StateComponents/AttackStateComponent.h"
#include "Components/Rigidbody.h"
#include "Components/StateComponent.h"
#include "Components/SFXComponent.h"

struct EnactAnimationActionSystem : public ISystem<EnactActionComponent, AnimatedActionComponent, Animator, RenderProperties, RenderComponent<RenderType>>
{
  static void DoTick(float dt);
  static void PlayAnimation(EntityID entity, const std::string& animation, bool looped, float playSpeed, bool forceAnimRestart, bool facingRight);

};

struct EnactAttackActionSystem : public ISystem<EnactActionComponent, AnimatedActionComponent, AttackActionComponent, Animator>
{
  static void DoTick(float dt);

};

struct EnactGrappleActionSystem : public ISystem<EnactActionComponent, GrappleActionComponent, Rigidbody>
{
  static void DoTick(float dt);

};

struct EnactActionMovementSystem : public ISystem<EnactActionComponent, MovingActionComponent, Rigidbody>
{
  static void DoTick(float dt);

};

struct EnactActionDamageSystem : public ISystem<EnactActionComponent, ReceivedDamageAction, StateComponent, SFXComponent>
{
  static void DoTick(float dt);

};

struct EnactGrappledSystem : public ISystem<EnactActionComponent, ReceivedGrappleAction, StateComponent, Rigidbody>
{
  static void DoTick(float dt);

};

struct CleanUpActionSystem : public ISystem<EnactActionComponent>
{
  static void PostUpdate();

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
    PROFILE_FUNCTION();
    EnactAnimationActionSystem::DoTick(dt);
    EnactAttackActionSystem::DoTick(dt);
    EnactGrappleActionSystem::DoTick(dt);
    EnactActionMovementSystem::DoTick(dt);
    EnactActionDamageSystem::DoTick(dt);
    EnactGrappledSystem::DoTick(dt);
  }
};
