#pragma once
#include "Core/ECS/IComponent.h"

#include "Components/Rigidbody.h"
#include "Components/StateComponent.h"
#include "Components/Actors/GameActor.h"

#include <sstream>

struct AnimatedActionComponent : public IComponent, ISerializable
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

  void Serialize(std::ostream& os) const override
  {
    Serializer<bool>::Serialize(os, isFacingRight);
    Serializer<bool>::Serialize(os, isLoopedAnimation);
    Serializer<bool>::Serialize(os, forceAnimRestart);
    Serializer<float>::Serialize(os, playSpeed);
    Serializer<std::string>::Serialize(os, animation);
    Serializer<bool>::Serialize(os, complete);
  }
  void Deserialize(std::istream& is) override
  {
    Serializer<bool>::Deserialize(is, isFacingRight);
    Serializer<bool>::Deserialize(is, isLoopedAnimation);
    Serializer<bool>::Deserialize(is, forceAnimRestart);
    Serializer<float>::Deserialize(is, playSpeed);
    Serializer<std::string>::Deserialize(is, animation);
    Serializer<bool>::Deserialize(is, complete);
  }

  std::string Log() override
  {
    std::stringstream ss;
    ss << "AnimationActionComponent: \n";
    ss << "\tIs facing right: " << isFacingRight << "\n";
    ss << "\tIs looped action: " << isLoopedAnimation << "\n";
    ss << "\tforceAnimRestart: " << forceAnimRestart << "\n";
    ss << "\tPlay Speed: " << playSpeed << "\n";
    ss << "\tAnimation name: " << animation << "\n";
    ss << "\tComplete: " << complete << "\n";
    return ss.str();
  }

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
struct HittableState : public IComponent, ISerializable
{
  bool canBlock = true;
  bool inKnockdown = false;

  void Serialize(std::ostream& os) const override
  {
    Serializer<bool>::Serialize(os, canBlock);
    Serializer<bool>::Serialize(os, inKnockdown);
  }
  void Deserialize(std::istream& is) override
  {
    Serializer<bool>::Deserialize(is, canBlock);
    Serializer<bool>::Deserialize(is, inKnockdown);
  }

  std::string Log() override
  {
    std::stringstream ss;
    ss << "HittableState: \n";
    ss << "\tCan block: " << canBlock << "\n";
    ss << "\tIn knockdown: " << inKnockdown << "\n";
    return ss.str();
  }
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

struct AttackActionComponent : public IComponent, ISerializable
{
  ActionState type = ActionState::NONE;

  void Serialize(std::ostream& os) const override
  {
    Serializer<ActionState>::Serialize(os, type);
  }
  void Deserialize(std::istream& is) override
  {
    Serializer<ActionState>::Deserialize(is, type);
  }

  std::string Log() override
  {
    std::stringstream ss;
    ss << "AttackActionComponent: \n";
    ss << "\tAction type: " << (int)type << "\n";
    return ss.str();
  }

};

struct GrappleActionComponent : public IComponent
{
  void OnRemove(const EntityID& entity) override
  {
    if (ComponentArray<Rigidbody>::Get().HasComponent(entity))
      ComponentArray<Rigidbody>::Get().GetComponent(entity).ignoreDynamicColliders = false;
  }
};

struct MovingActionComponent : public IComponent, ISerializable
{
  //
  bool horizontalMovementOnly = false;
  Vector2<float> velocity;

  void Serialize(std::ostream& os) const override
  {
    Serializer<bool>::Serialize(os, horizontalMovementOnly);
    os << velocity;
  }
  void Deserialize(std::istream& is) override
  {
    Serializer<bool>::Deserialize(is, horizontalMovementOnly);
    is >> velocity;
  }

  std::string Log() override
  {
    std::stringstream ss;
    ss << "MovingActionComponent: \n";
    ss << "\tHorizontal movement only: " << horizontalMovementOnly << "\n";
    ss << "\tVelocity: " << velocity.x << " " << velocity.y << "\n";
    return ss.str();
  }

};

struct ReceivedDamageAction : public IComponent, ISerializable
{
  int damageAmount;
  bool isBlocking = false;
  bool fromGrapple = false;
  bool isKillingBlow = false;

  void Serialize(std::ostream& os) const override
  {
    Serializer<int>::Serialize(os, damageAmount);
    Serializer<bool>::Serialize(os, isBlocking);
    Serializer<bool>::Serialize(os, fromGrapple);
    Serializer<bool>::Serialize(os, isKillingBlow);
  }
  void Deserialize(std::istream& is) override
  {
    Serializer<int>::Deserialize(is, damageAmount);
    Serializer<bool>::Deserialize(is, isBlocking);
    Serializer<bool>::Deserialize(is, fromGrapple);
    Serializer<bool>::Deserialize(is, isKillingBlow);
  }

  std::string Log() override
  {
    std::stringstream ss;
    ss << "ReceivedDamageAction: \n";
    ss << "\tDamage Amount: " << damageAmount << "\n";
    ss << "\tIs Blocking: " << isBlocking << "\n";
    ss << "\tFrom Grapple: " << fromGrapple << "\n";
    ss << "\tIs Killing blow: " << isKillingBlow << "\n";
    return ss.str();
  }
};

struct ReceivedGrappleAction : public IComponent, ISerializable
{
  void OnRemove(const EntityID& entity) override
  {
    if (ComponentArray<Rigidbody>::Get().HasComponent(entity))
      ComponentArray<Rigidbody>::Get().GetComponent(entity).ignoreDynamicColliders = false;
  }
  int damageAmount;
  bool isKillingBlow = false;
  int damageAndKnockbackDelay = 0;

  void Serialize(std::ostream& os) const override
  {
    Serializer<int>::Serialize(os, damageAmount);
    Serializer<bool>::Serialize(os, isKillingBlow);
    Serializer<int>::Serialize(os, damageAndKnockbackDelay);
  }
  void Deserialize(std::istream& is) override
  {
    Serializer<int>::Deserialize(is, damageAmount);
    Serializer<bool>::Deserialize(is, isKillingBlow);
    Serializer<int>::Deserialize(is, damageAndKnockbackDelay);
  }

  std::string Log() override
  {
    std::stringstream ss;
    ss << "ReceivedGrappleAction: \n";
    ss << "\tDamage Amount: " << damageAmount << "\n";
    ss << "\tIs Killing blow: " << isKillingBlow << "\n";
    ss << "\tDamage and knockback delay: " << damageAndKnockbackDelay << "\n";
    return ss.str();
  }


};

struct DashingAction : public IComponent, ISerializable
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

  void Serialize(std::ostream& os) const override
  {
    Serializer<float>::Serialize(os, dashSpeed);
  }
  void Deserialize(std::istream& is) override
  {
    Serializer<float>::Deserialize(is, dashSpeed);
  }

  std::string Log() override
  {
    std::stringstream ss;
    ss << "DashingAction: \n";
    ss << "\tDash Speed: " << dashSpeed << "\n";
    return ss.str();
  }
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

struct TimedActionComponent : public IComponent, ISerializable
{
  //!
  float playTime = 0.0f;
  int currFrame = 0;
  int totalFrames = 0;

  bool cancelled = false;

  void Serialize(std::ostream& os) const override
  {
    Serializer<float>::Serialize(os, playTime);
    Serializer<int>::Serialize(os, currFrame);
    Serializer<int>::Serialize(os, totalFrames);
    Serializer<bool>::Serialize(os, cancelled);
  }
  void Deserialize(std::istream& is) override
  {
    Serializer<float>::Deserialize(is, playTime);
    Serializer<int>::Deserialize(is, currFrame);
    Serializer<int>::Deserialize(is, totalFrames);
    Serializer<bool>::Deserialize(is, cancelled);
  }

  std::string Log() override
  {
    std::stringstream ss;
    ss << "TimedActionComponent: \n";
    ss << "\tPlay Time: " << playTime << "\n";
    ss << "\tCurrent Frame: " << currFrame << "\n";
    ss << "\tTotal frames: " << totalFrames << "\n";
    ss << "\tCancelled: " << cancelled << "\n";
    return ss.str();
  }
};

struct WaitForAnimationComplete : public IComponent {};

//! Fully transitions to jumping state
struct WaitingForJumpAirborne : public IComponent {};


//! Component for pushing player away from other player when pressuring on the wall - plz move later
struct WallPushComponent : public IComponent, ISerializable
{
  void OnRemove(const EntityID& entity) override;

  float pushAmount = 0.0f;
  float amountPushed = 0.0f;
  float velocity = 0.0f;

  void Serialize(std::ostream& os) const override
  {
    Serializer<float>::Serialize(os, pushAmount);
    Serializer<float>::Serialize(os, amountPushed);
    Serializer<float>::Serialize(os, velocity);
  }
  void Deserialize(std::istream& is) override
  {
    Serializer<float>::Deserialize(is, pushAmount);
    Serializer<float>::Deserialize(is, amountPushed);
    Serializer<float>::Deserialize(is, velocity);
  }


  std::string Log() override
  {
    std::stringstream ss;
    ss << "WallPushComponent: \n";
    ss << "\tPush amount: " << pushAmount << "\n";
    ss << "\tAmount pushed: " << amountPushed << "\n";
    ss << "\tSpeed: " << velocity << "\n";
    return ss.str();
  }
};
