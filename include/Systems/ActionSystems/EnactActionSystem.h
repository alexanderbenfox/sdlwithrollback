#pragma once
#include "Systems/ISystem.h"

#include "Components/ActionComponents.h"
#include "Components/Animator.h"
#include "Components/GameActor.h"
#include "Components/RenderComponent.h"
#include "Components/Hurtbox.h"

struct EnactAnimationActionSystem : public ISystem<EnactActionComponent, AnimatedActionComponent, Animator, RenderProperties, RenderComponent<RenderType>, Hurtbox, GameActor>
{
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      AnimatedActionComponent* action = std::get<AnimatedActionComponent*>(tuple.second);
      Animator* animator = std::get<Animator*>(tuple.second);
      RenderProperties* properties = std::get<RenderProperties*>(tuple.second);
      RenderComponent<RenderType>* renderer = std::get<RenderComponent<RenderType>*>(tuple.second);
      Hurtbox* hurtbox = std::get<Hurtbox*>(tuple.second);
      GameActor* actor = std::get<GameActor*>(tuple.second);

      if (animator->AnimationLib() && animator->AnimationLib()->GetAnimation(action->animation))
      {
        Animation* actionAnimation = animator->Play(action->animation, action->isLoopedAnimation, action->playSpeed, action->forceAnimRestart);
        properties->horizontalFlip = !action->isFacingRight;
        properties->offset = -animator->AnimationLib()->GetRenderOffset(action->animation, !action->isFacingRight, (int)std::floor(hurtbox->unscaledRect.Width()));
        if (actionAnimation)
        {
          // render from the sheet of the new animation
          renderer->SetRenderResource(actionAnimation->GetSheetTexture<RenderType>());
          renderer->sourceRect = actionAnimation->GetFrameSrcRect(0);
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
    for (auto tuple : Tuples)
    {
      AnimatedActionComponent* actionData = std::get<AnimatedActionComponent*>(tuple.second);
      AttackActionComponent* action = std::get<AttackActionComponent*>(tuple.second);
      Animator* animator = std::get<Animator*>(tuple.second);

      if (animator->AnimationLib()->GetAnimation(actionData->animation) && animator->AnimationLib()->GetEventList(actionData->animation))
      {
        action->Owner()->AddComponent<AttackStateComponent>();
        action->Owner()->GetComponent<AttackStateComponent>()->Init(animator->AnimationLib()->GetAnimation(actionData->animation), animator->AnimationLib()->GetEventList(actionData->animation));
      }
    }
  }
};

struct EnactGrappleActionSystem : public ISystem<EnactActionComponent, GrappleActionComponent, Rigidbody>
{
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      Rigidbody* rb = std::get<Rigidbody*>(tuple.second);
      rb->ignoreDynamicColliders = true;
    }
  }
};

#include "Components/Rigidbody.h"

struct EnactActionMovementSystem : public ISystem<EnactActionComponent, MovingActionComponent, Rigidbody>
{
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      MovingActionComponent* action = std::get<MovingActionComponent*>(tuple.second);
      Rigidbody* rb = std::get<Rigidbody*>(tuple.second);

      rb->_vel = action->velocity;
    }
  }
};

#include "Components/StateComponent.h"
#include "Components/SFXComponent.h"

struct EnactActionDamageSystem : public ISystem<EnactActionComponent, ReceivedDamageAction, StateComponent, SFXComponent>
{
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      ReceivedDamageAction* action = std::get<ReceivedDamageAction*>(tuple.second);
      StateComponent* state = std::get<StateComponent*>(tuple.second);

      if (!state->invulnerable)
      {
        state->hp -= action->damageAmount;
      }

      SFXComponent* sfx = std::get<SFXComponent*>(tuple.second);
      if (action->isBlocking)
        sfx->ShowBlockSparks();
      else if(!action->fromGrapple)
        sfx->ShowHitSparks();

      if (!action->fromGrapple)
      {
        GameManager::Get().ActivateHitStop(7);
      }
    }
  }
};

struct EnactGrappledSystem : public ISystem<EnactActionComponent, ReceivedGrappleAction, StateComponent, Rigidbody>
{
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      ReceivedGrappleAction* action = std::get<ReceivedGrappleAction*>(tuple.second);
      StateComponent* state = std::get<StateComponent*>(tuple.second);
      Rigidbody* rb = std::get<Rigidbody*>(tuple.second);

      state->thrownThisFrame = false;
      rb->ignoreDynamicColliders = true;

      state->Owner()->AddComponent<TimedActionComponent>();
      state->Owner()->GetComponent<TimedActionComponent>()->totalFrames = action->damageAndKnockbackDelay;
    }
  }
};

struct CleanUpActionSystem : public ISystem<EnactActionComponent, StateComponent>
{
  static void PostUpdate()
  {
    std::vector<Entity*> entities;
    for (auto tuple : Tuples)
    {
      EnactActionComponent* action = std::get<EnactActionComponent*>(tuple.second);
      StateComponent* state = std::get<StateComponent*>(tuple.second);

      entities.push_back(action->Owner());
    }

    for (auto entity : entities)
    {
      entity->RemoveComponent<EnactActionComponent>();
      // we want to be listening for a new action now
      entity->AddComponent<InputListenerComponent>();
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
