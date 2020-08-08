#pragma once
#include "Systems/ISystem.h"

#include "Components/ActionComponents.h"
#include "Components/GameActor.h"
#include "Components/Rigidbody.h"
#include "Components/Animator.h"

struct TimedActionSystem : public ISystem<TimedActionComponent, GameActor>
{
  static void DoTick(float dt);
};

struct HandleInputGrappledActionSystem : public ISystem<InputListenerComponent, ReceivedGrappleAction, GameActor, StateComponent>
{
  static void DoTick(float dt);
};

struct HandleDashUpdateSystem : public ISystem<DashingAction, Rigidbody, TimedActionComponent>
{
  static void DoTick(float dt);
};

struct HandleInputJump : public ISystem<JumpingAction, Rigidbody, GameActor>
{
  static void DoTick(float dt);
};

struct HandleInputCrouch : public ISystem<InputListenerComponent, CrouchingAction, Rigidbody, GameActor>
{
  static void DoTick(float dt);
};

struct HandleInputGrappling : public ISystem<InputListenerComponent, GameActor, StateComponent>
{
  static void DoTick(float dt);
};

struct CheckForMove : public ISystem<InputListenerComponent, AbleToWalk, GameActor, Rigidbody, StateComponent>
{
  static void DoTick(float dt);
};

struct CheckForJump : public ISystem<InputListenerComponent, AbleToJump, GameActor, Rigidbody, StateComponent>
{
  static void DoTick(float dt);
};

struct CheckForBeginCrouching : public ISystem<InputListenerComponent, AbleToCrouch, GameActor, Rigidbody, StateComponent>
{
  static void DoTick(float dt);
};

struct CheckHitThisFrameSystem : public ISystem<InputListenerComponent, HittableState, GameActor, StateComponent>
{
  static void DoTick(float dt);
};

struct CheckSpecialAttackInputSystem : public ISystem<InputListenerComponent, AbleToSpecialAttackState, GameActor, StateComponent>
{
  static void DoTick(float dt);
};

struct CheckDashSystem : public ISystem<InputListenerComponent, AbleToDash, GameActor, StateComponent, Animator, Rigidbody>
{
  static void DoTick(float dt);
};

struct CheckAttackInputSystem : public ISystem<InputListenerComponent, AbleToAttackState, GameActor, StateComponent>
{
  static void DoTick(float dt);
};

struct CheckHitGroundCancel : public ISystem<CancelOnHitGround, JumpingAction, Rigidbody, GameActor>
{
  static void DoTick(float dt);
};

struct ListenForAirborneSystem : public ISystem<WaitingForJumpAirborne, Rigidbody, GameActor>
{
  static void DoTick(float dt);
};

struct TransitionToNeutralSystem : public ISystem<InputListenerComponent, TransitionToNeutral, StateComponent, GameActor>
{
  static void DoTick(float dt);
};

struct CheckKnockdownComplete : public ISystem<InputListenerComponent, TransitionToKnockdownGround, StateComponent, GameActor>
{
  static void DoTick(float dt);
};

struct CheckKnockdownOTG : public ISystem<InputListenerComponent, TransitionToKnockdownGroundOTG, StateComponent, Rigidbody>
{
  static void DoTick(float dt);
};

struct CheckCrouchingFollowUp : public ISystem<InputListenerComponent, TransitionToCrouching, StateComponent, GameActor>
{
  static void DoTick(float dt);
};

struct StateTransitionAggregate
{
  static void Check(Entity* entity)
  {
    CheckForMove::Check(entity);
    CheckForJump::Check(entity);
    CheckForBeginCrouching::Check(entity);
    CheckDashSystem::Check(entity);
    CheckHitThisFrameSystem::Check(entity);

    CheckSpecialAttackInputSystem::Check(entity);
    CheckAttackInputSystem::Check(entity);

    CheckHitGroundCancel::Check(entity);

    ListenForAirborneSystem::Check(entity);
    TransitionToNeutralSystem::Check(entity);

    CheckKnockdownComplete::Check(entity);

    CheckKnockdownOTG::Check(entity);

    CheckCrouchingFollowUp::Check(entity);
  }

  static void DoTick(float dt)
  {
    CheckHitThisFrameSystem::DoTick(dt);
    CheckSpecialAttackInputSystem::DoTick(dt);
    CheckAttackInputSystem::DoTick(dt);
    CheckForJump::DoTick(dt);
    CheckForBeginCrouching::DoTick(dt);
    CheckDashSystem::DoTick(dt);
    CheckCrouchingFollowUp::DoTick(dt);
    CheckForMove::DoTick(dt);


    CheckHitGroundCancel::DoTick(dt);

    ListenForAirborneSystem::DoTick(dt);
    

    CheckKnockdownComplete::DoTick(dt);

    CheckKnockdownOTG::DoTick(dt);
    TransitionToNeutralSystem::DoTick(dt);
  }
};

struct HandleUpdateAggregate
{
  static void Check(Entity* entity)
  {
    TimedActionSystem::Check(entity);
    HandleInputJump::Check(entity);
    HandleInputCrouch::Check(entity);
    HandleInputGrappledActionSystem::Check(entity);
    HandleDashUpdateSystem::Check(entity);
    HandleInputGrappling::Check(entity);
  }

  static void DoTick(float dt)
  {
    HandleInputJump::DoTick(dt);
    HandleInputCrouch::DoTick(dt);
    HandleInputGrappledActionSystem::DoTick(dt);
    HandleDashUpdateSystem::DoTick(dt);
    HandleInputGrappling::DoTick(dt);
  }
};
