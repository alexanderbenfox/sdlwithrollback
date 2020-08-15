#pragma once
#include "Core/ECS/IComponent.h"

#include "Components/Rigidbody.h"
#include "Components/StateComponent.h"
#include "Components/Actors/GameActor.h"

struct AnimatedActionComponent : public IComponent
{
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
struct EnactActionComponent : public IComponent {};

//! Marks the entity as available to check for another action
struct InputListenerComponent : public IComponent {};

// Marks entity hittable
struct HittableState : public IComponent
{
  bool canBlock = true;
  bool inKnockdown = false;
};

struct AbleToAttackState : public IComponent {};

struct AbleToSpecialAttackState : public IComponent {};

struct AbleToDash : public IComponent {};

struct AbleToJump : public IComponent {};

struct AbleToWalkLeft : public IComponent {};

struct AbleToWalkRight : public IComponent {};

struct AbleToCrouch : public IComponent {};

struct AbleToReturnToNeutral : public IComponent {};

//! Follow up action component - player hit the ground but other player can still OTG
struct TransitionToKnockdownGroundOTG : public IComponent {};

//! Follow up action component - player cannot otg
struct TransitionToKnockdownGround : public IComponent {};

struct TransitionToNeutral : public IComponent {};

struct TransitionToCrouching : public IComponent {};

//! Cancel actions

struct CancelOnHitGround : public IComponent {};

struct CancelOnDash : public IComponent {};

struct CancelOnJump : public IComponent {};

struct CancelOnSpecial : public IComponent {};

// this component will work in conjunction with the HasTargetCombo component (and other action mapping components of that type)
struct CancelOnNormal : public IComponent {};

//! Components that describe the enacting parameters for a given action type

struct AttackActionComponent : public IComponent
{
  ActionState type = ActionState::NONE;
};

struct GrappleActionComponent : public IComponent
{
  void OnRemove(const EntityID& entity) override
  {
    if (ComponentArray<Rigidbody>::Get().HasComponent(entity))
      ComponentArray<Rigidbody>::Get().GetComponent(entity).ignoreDynamicColliders = false;
  }
};

struct MovingActionComponent : public IComponent
{
  //
  bool horizontalMovementOnly = false;
  Vector2<float> velocity;
};

struct ReceivedDamageAction : public IComponent
{
  int damageAmount;
  bool isBlocking = false;
  bool fromGrapple = false;
  bool isKillingBlow = false;
};

struct ReceivedGrappleAction : public IComponent
{
  void OnRemove(const EntityID& entity) override
  {
    if (ComponentArray<Rigidbody>::Get().HasComponent(entity))
      ComponentArray<Rigidbody>::Get().GetComponent(entity).ignoreDynamicColliders = false;
  }
  int damageAmount;
  bool isKillingBlow = false;
  int damageAndKnockbackDelay = 0;
};

struct DashingAction : public IComponent
{
  void OnAdd(const EntityID& entity) override
  {
    ComponentArray<GameActor>::Get().GetComponent(entity).forceNewInputOnNextFrame = true;
  }
  void OnRemove(const EntityID& entity) override
  {
    ComponentArray<GameActor>::Get().GetComponent(entity).forceNewInputOnNextFrame = true;
  }

  float dashSpeed = 0.0f;
};

struct JumpingAction : public IComponent
{
  void OnAdd(const EntityID& entity) override
  {
    ComponentArray<GameActor>::Get().GetComponent(entity).forceNewInputOnNextFrame = true;
  }
  void OnRemove(const EntityID& entity) override
  {
    ComponentArray<GameActor>::Get().GetComponent(entity).forceNewInputOnNextFrame = true;
  }
};

struct CrouchingAction : public IComponent
{
  void OnAdd(const EntityID& entity) override
  {
    ComponentArray<GameActor>::Get().GetComponent(entity).forceNewInputOnNextFrame = true;
  }
  void OnRemove(const EntityID& entity) override
  {
    ComponentArray<GameActor>::Get().GetComponent(entity).forceNewInputOnNextFrame = true;
  }
};

//! Components that describe the completion parameters for a given action type

struct TimedActionComponent : public IComponent
{
  //!
  float playTime = 0.0f;
  int currFrame = 0;
  int totalFrames = 0;

  bool cancelled = false;
};

struct WaitForAnimationComplete : public IComponent {};

//! Fully transitions to jumping state
struct WaitingForJumpAirborne : public IComponent {};

// for input state
#include "Components/InputHandlers/InputBuffer.h"


//! Character type identifier components??
// Works with the CancelOnNormal component

struct HasTargetCombo : public IComponent
{
  //! Defines which normals can be cancelled into each other
  std::unordered_map<ActionState, InputState> links;

};


//! Component for pushing player away from other player when pressuring on the wall - plz move later
struct WallPushComponent : public IComponent
{
  WallPushComponent();
  void OnRemove(const EntityID& entity) override;

  float pushAmount;
  float amountPushed = 0.0f;
  float velocity;
};
