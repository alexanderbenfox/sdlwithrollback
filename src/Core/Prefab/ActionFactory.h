#pragma once
#include "Components/StateComponent.h"
#include "Components/Animator.h"
#include "Components/Actors/GameActor.h"

//! static utility class for adding appropriate components to entity
class ActionFactory
{
public:
  static void SetAerialState(const EntityID& entity);

  static void SetCrouchingState(const EntityID& entity, StateComponent* state);

  static void SetKnockdownAirborne(const EntityID& entity, StateComponent* state);

  static void SetKnockdownGroundOTG(const EntityID& entity, StateComponent* state);

  static void SetKnockdownGroundInvincible(const EntityID& entity, StateComponent* state);

  static void SetBlockStunAction(const EntityID& entity, StateComponent* state, bool crouching);

  static void SetHitStunAction(const EntityID& entity, StateComponent* state, bool crouching);

  static void SetGrappledAction(const EntityID& entity, StateComponent* state);

  static void SetAttackAction(const EntityID& entity, StateComponent* state, const std::string& attackName, ActionState actionType);

  static void SetDashAction(const EntityID& entity, StateComponent* state, Animator* animator, bool dashDirectionForward);

  static void GoToNeutralAction(const EntityID& entity, StateComponent* state);

  static void GoToWalkLeftAction(const EntityID& entity, GameActor* actor, StateComponent* state, const Vector2<float>& mvmt);

  static void GoToWalkRightAction(const EntityID& entity, GameActor* actor, StateComponent* state, const Vector2<float>& mvmt);

  static void ResetActionComponents(const EntityID& entity);

  static void RemoveTransitionComponents(const EntityID& entity);

  static void DisableAbility(const EntityID& entity);

  static void EnableAbility(const EntityID& entity);

  //! Hacky utility for checking cancels on ryu character right now.... will remove once the character system is figured out
  static bool ActorDidSpecialInputRyu(GameActor* actor, StateComponent* state);

};