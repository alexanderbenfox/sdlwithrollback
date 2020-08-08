#pragma once
#include "Components/IComponent.h"
#include "StateMachine/StateEnums.h"

#include "Components/Rigidbody.h"
#include "Components/StateComponent.h"

struct AnimatedActionComponent : public IComponent
{
  AnimatedActionComponent(std::shared_ptr<Entity> e) : IComponent(e) {}

  //! State of player when starting this action
  //!
  bool isFacingRight = false;
  //!
  bool isLoopedAnimation = false;

  //! Initialization parameters
  //!
  bool forceAnimRestart = false;
  //!
  float playSpeed = 1.0f;
  //!
  std::string animation = "";

  //! State of this action
  bool complete = false;

};

template <> struct ComponentInitParams<AnimatedActionComponent>
{
  bool isFacingRight = false;
  bool isLoopedAnimation = false;
  bool forceAnimRestart = false;
  float playSpeed = 1.0f;
  std::string animation = "";

  static void Init(AnimatedActionComponent& component, const ComponentInitParams<AnimatedActionComponent>& params)
  {
    component.isFacingRight = params.isFacingRight;
    component.isLoopedAnimation = params.isLoopedAnimation;
    component.forceAnimRestart = params.forceAnimRestart;
    component.playSpeed = params.playSpeed;
    component.animation = params.animation;
  }
};

//! Empty components for indicating which system should be run for handling input
struct EnactActionComponent : public IComponent
{
  EnactActionComponent(std::shared_ptr<Entity> e) : IComponent(e) {}
};

//! Marks the entity as available to check for another action
struct InputListenerComponent : public IComponent
{
  InputListenerComponent(std::shared_ptr<Entity> e) : IComponent(e) {}
};

// Marks entity hittable
struct HittableState : public IComponent
{
  HittableState(std::shared_ptr<Entity> e) : IComponent(e) {}
  bool canBlock = true;
  bool inKnockdown = false;
};

struct AbleToAttackState : public IComponent
{
  AbleToAttackState(std::shared_ptr<Entity> e) : IComponent(e) {}
};

struct AbleToSpecialAttackState : public IComponent
{
  AbleToSpecialAttackState(std::shared_ptr<Entity> e) : IComponent(e) {}
};

struct AbleToDash : public IComponent
{
  AbleToDash(std::shared_ptr<Entity> e) : IComponent(e) {}
};

struct AbleToJump : public IComponent
{
  AbleToJump(std::shared_ptr<Entity> e) : IComponent(e) {}
};

struct AbleToWalk : public IComponent
{
  AbleToWalk(std::shared_ptr<Entity> e) : IComponent(e) {}
};

struct AbleToCrouch : public IComponent
{
  AbleToCrouch(std::shared_ptr<Entity> e) : IComponent(e) {}
};

//! Follow up action component - player hit the ground but other player can still OTG
struct TransitionToKnockdownGroundOTG : public IComponent
{
  TransitionToKnockdownGroundOTG(std::shared_ptr<Entity> e) : IComponent(e) {}
};

//! Follow up action component - player cannot otg
struct TransitionToKnockdownGround : public IComponent
{
  TransitionToKnockdownGround(std::shared_ptr<Entity> e) : IComponent(e) {}
};

struct TransitionToNeutral : public IComponent
{
  TransitionToNeutral(std::shared_ptr<Entity> e) : IComponent(e) {}
};

struct TransitionToCrouching : public IComponent
{
  TransitionToCrouching(std::shared_ptr<Entity> e) : IComponent(e) {}
};

//! Cancel actions

struct CancelOnHitGround : public IComponent
{
  CancelOnHitGround(std::shared_ptr<Entity> e) : IComponent(e) {}
};

struct CancelOnDash : public IComponent
{
  CancelOnDash(std::shared_ptr<Entity> e) : IComponent(e) {}
};

struct CancelOnJump : public IComponent
{
  CancelOnJump(std::shared_ptr<Entity> e) : IComponent(e) {}
};

//! Components that describe the enacting parameters for a given action type

struct AttackActionComponent : public IComponent
{
  AttackActionComponent(std::shared_ptr<Entity> e) : IComponent(e) {}
  ActionState type = ActionState::NONE;
};

struct GrappleActionComponent : public IComponent
{
  GrappleActionComponent(std::shared_ptr<Entity> e) : IComponent(e) {}
  ~GrappleActionComponent()
  {
    if (auto rb = _owner->GetComponent<Rigidbody>())
      rb->ignoreDynamicColliders = false;
  }
};

struct MovingActionComponent : public IComponent
{
  MovingActionComponent(std::shared_ptr<Entity> e) : IComponent(e) {}

  Vector2<float> velocity;
};

struct ReceivedDamageAction : public IComponent
{
  ReceivedDamageAction(std::shared_ptr<Entity> e) : IComponent(e) {}

  int damageAmount;
  bool isBlocking = false;
  bool fromGrapple = false;
  bool isKillingBlow = false;
};

struct ReceivedGrappleAction : public IComponent
{
  ReceivedGrappleAction(std::shared_ptr<Entity> e) : IComponent(e) {}
  ~ReceivedGrappleAction()
  {
    if (auto rb = _owner->GetComponent<Rigidbody>())
      rb->ignoreDynamicColliders = false;
  }
  int damageAmount;
  bool isKillingBlow = false;
  int damageAndKnockbackDelay = 0;
};

struct DashingAction : public IComponent
{
  DashingAction(std::shared_ptr<Entity> e) : IComponent(e)
  {
    _owner->GetComponent<StateComponent>()->onNewState = true;
  }
  ~DashingAction()
  {
    _owner->GetComponent<StateComponent>()->onNewState = true;
  }
  float dashSpeed = 0.0f;
};

struct JumpingAction : public IComponent
{
  JumpingAction(std::shared_ptr<Entity> e) : IComponent(e)
  {
    _owner->GetComponent<StateComponent>()->onNewState = true;
  }
  ~JumpingAction()
  {
    _owner->GetComponent<StateComponent>()->onNewState = true;
  }
};

struct CrouchingAction : public IComponent
{
  CrouchingAction(std::shared_ptr<Entity> e) : IComponent(e)
  {
    _owner->GetComponent<StateComponent>()->onNewState = true;
  }
  ~CrouchingAction()
  {
    _owner->GetComponent<StateComponent>()->onNewState = true;
  }
};

//! Components that describe the completion parameters for a given action type

struct TimedActionComponent : public IComponent
{
  TimedActionComponent(std::shared_ptr<Entity> e) : IComponent(e) {}

  //!
  float playTime = 0.0f;
  int currFrame = 0;
  int totalFrames = 0;

  bool cancelled = false;
};

struct WaitForAnimationComplete : public IComponent
{
  WaitForAnimationComplete(std::shared_ptr<Entity> e) : IComponent(e) {}
};

//! Fully transitions to jumping state
struct WaitingForJumpAirborne : public IComponent
{
  WaitingForJumpAirborne(std::shared_ptr<Entity> e) : IComponent(e) {}
};
