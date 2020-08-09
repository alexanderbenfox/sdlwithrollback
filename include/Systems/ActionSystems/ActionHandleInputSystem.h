#pragma once
#include "Systems/ISystem.h"

#include "Components/ActionComponents.h"
#include "Components/Rigidbody.h"
#include "Components/Animator.h"
#include "Components/Actors/GameActor.h"

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

struct HandleInputGrappling : public ISystem<InputListenerComponent, GrappleActionComponent, GameActor, StateComponent>
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

struct CheckForFalling : public ISystem<InputListenerComponent, AbleToJump, GameActor, Rigidbody, StateComponent>
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

//! Kind of a weird system for cancelling out of action before it occurs
struct CheckGrappleCancelOnHit : public IMultiSystem<SysComponents<GrappleActionComponent, StateComponent>, SysComponents<ReceivedGrappleAction, Transform, StateComponent>>
{
  static void DoTick(float dt);
};

struct ListenForAirborneSystem : public ISystem<WaitingForJumpAirborne, Rigidbody, GameActor>
{
  static void DoTick(float dt);
};

struct TransitionToNeutralSystem : public ISystem<InputListenerComponent, TransitionToNeutral, StateComponent, GameActor, Rigidbody>
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

//______________________________________________________________________________
// cancel here?

struct HitGroundCancelActionSystem : public ISystem<CancelOnHitGround, JumpingAction, Rigidbody, GameActor>
{
  static void DoTick(float dt);
};

struct SpecialMoveCancelActionSystem : public ISystem<CancelOnSpecial, GameActor, StateComponent>
{
  static void DoTick(float dt);
};

struct TargetComboCancelActionSystem : public ISystem<CancelOnNormal, HasTargetCombo, GameActor, StateComponent>
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
    CheckForFalling::Check(entity);

    CheckSpecialAttackInputSystem::Check(entity);
    CheckAttackInputSystem::Check(entity);

    HitGroundCancelActionSystem::Check(entity);
    SpecialMoveCancelActionSystem::Check(entity);
    TargetComboCancelActionSystem::Check(entity);

    ListenForAirborneSystem::Check(entity);
    TransitionToNeutralSystem::Check(entity);

    CheckKnockdownComplete::Check(entity);

    CheckKnockdownOTG::Check(entity);

    CheckCrouchingFollowUp::Check(entity);

    CheckGrappleCancelOnHit::Check(entity);
  }

  static void DoTick(float dt)
  {
    // determine passive state before evaluating if entity got hit
    CheckForFalling::DoTick(dt);
    CheckForJump::DoTick(dt);
    CheckForBeginCrouching::DoTick(dt);
    CheckCrouchingFollowUp::DoTick(dt);
    ListenForAirborneSystem::DoTick(dt);

    // determine action cancels before doing action evaluation
    HitGroundCancelActionSystem::DoTick(dt);
    SpecialMoveCancelActionSystem::DoTick(dt);
    TargetComboCancelActionSystem::DoTick(dt);

    // Because of this a 5 frame start up normal will beat a 5 frame start up throw... is that right?? 
    CheckGrappleCancelOnHit::DoTick(dt);

    // determine active states - will cancel any subsequent action selection (order matters here)
    CheckHitThisFrameSystem::DoTick(dt);
    CheckSpecialAttackInputSystem::DoTick(dt);
    CheckAttackInputSystem::DoTick(dt);
    CheckDashSystem::DoTick(dt);
    CheckForMove::DoTick(dt);

    // another set of actions that cancel override following actions
    CheckKnockdownComplete::DoTick(dt);
    CheckKnockdownOTG::DoTick(dt);
  }
};

struct HandleUpdateAggregate
{
  static void Check(Entity* entity)
  {
    // timed action system gets handled in scene update because it needs to go
    // after enact... maybe not appropriate to have in this aggregate
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
