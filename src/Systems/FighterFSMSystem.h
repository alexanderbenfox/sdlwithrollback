#pragma once
#include "Core/ECS/ISystem.h"
#include "Components/FighterFSMComponent.h"
#include "Components/Actors/GameActor.h"
#include "Components/StateComponent.h"
#include "Components/Rigidbody.h"
#include "Components/Animator.h"

//______________________________________________________________________________
struct FighterFSMSystem : public ISystem<FighterFSMComponent, GameActor, StateComponent, Rigidbody, Animator>
{
  static void DoTick(float dt);

private:
  // Build condition flags from current entity state
  static ConditionFlags EvaluateConditionFlags(
    const FighterFSMComponent& fsm, const GameActor& actor,
    const StateComponent& state, const Rigidbody& rb, const Animator& animator);

  // Evaluate transition rules, return target state (currentState if none matched)
  static FighterStateID EvaluateTransitions(
    EntityID entity, FighterFSMComponent& fsm, const ConditionFlags& flags,
    const GameActor& actor, StateComponent& state, const Rigidbody& rb);

  // Resolve which damage state to enter when hit/thrown
  static FighterStateID ResolveHitTarget(
    const FighterFSMComponent& fsm, const GameActor& actor,
    StateComponent& state, const Rigidbody& rb);

  // Execute one-time actions when entering a new state
  static void EnactState(
    EntityID entity, FighterFSMComponent& fsm, GameActor& actor,
    StateComponent& state, Rigidbody& rb, Animator& animator);

  // Advance timer, compute dash velocity, etc.
  static void UpdateCurrentState(
    float dt, EntityID entity, FighterFSMComponent& fsm,
    StateComponent& state, Rigidbody& rb);

  // Change to a new state, clean up old state
  static void TransitionTo(
    EntityID entity, FighterFSMComponent& fsm, FighterStateID target,
    StateComponent& state, Rigidbody& rb);
};
