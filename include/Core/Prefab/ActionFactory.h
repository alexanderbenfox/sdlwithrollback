#pragma once
#include "Components/StateComponent.h"
#include "Components/Animator.h"

//! static utility class for adding appropriate components to entity
class ActionFactory
{
public:
  static void SetKnockdownAirborne(Entity* entity, StateComponent* state);

  static void SetKnockdownGroundOTG(Entity* entity, StateComponent* state);

  static void SetKnockdownGroundInvincible(Entity* entity, StateComponent* state);

  static void SetBlockStunAction(Entity* entity, StateComponent* state, bool crouching);

  static void SetHitStunAction(Entity* entity, StateComponent* state, bool crouching);

  static void SetGrappledAction(Entity* entity, StateComponent* state);

  static void SetAttackAction(Entity* entity, StateComponent* state, const std::string& attackName, ActionState actionType);

  static void SetDashAction(Entity* entity, StateComponent* state, Animator* animator, bool dashDirectionForward);

  static void GoToNeutralAction(Entity* entity, StateComponent* state);

  static void ResetActionComponents(Entity* entity);

  static void RemoveTransitionComponents(Entity* entity);

  static void DisableAbility(Entity* entity);

  static void EnableAbility(Entity* entity);

  static void SetEntityDecided(Entity* entity) { _actionDecided.push_back(entity); }

  

  //! for the list of entities set to new actions, remove their 'waiting for action input' flag
  static void DisableActionListenerForEntities();
  

private:
  // when transitioning to a new action, remove InputListenerComponent so another action state does not overwrite it
  // the action has been decided for this frame
  static std::vector<Entity*> _actionDecided;

};