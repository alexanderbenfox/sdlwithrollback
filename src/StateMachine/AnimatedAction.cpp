#include "StateMachine/AnimatedAction.h"
#include "StateMachine/TimedAction.h"
#include "StateMachine/AttackAction.h"
#include "StateMachine/ActionUtil.h"

//______________________________________________________________________________
template <> IAction* StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::NONE>::GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context)
{
  LoopedAction<StanceState::CROUCHING, ActionState::NONE> followUp("Crouch", _facingRight);
  IAction* action = followUp.HandleInput(rawInput, context);
  if (action)
    return action;
  return new LoopedAction<StanceState::CROUCHING, ActionState::NONE>("Crouch", _facingRight);
}

//______________________________________________________________________________
template <> IAction* LoopedAction<StanceState::STANDING, ActionState::NONE>::HandleInput(const InputBuffer& rawInput, const StateComponent& context)
{
  bool facingRight = context.onLeftSide;
  IAction* nextAction = nullptr;

  // process attacks before hits so that if you press a button while attacked, you still get attacked
  if ((nextAction = GetAttacksFromNeutral<StanceState::STANDING>(rawInput, context))) return nextAction;

  if ((nextAction = CheckHits(rawInput.Latest(), context, true, false))) return nextAction;

  if (context.collision == CollisionSide::NONE)
    return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Falling", facingRight);

  //if you arent attacking, you can move forward, move backward, crouch, stand, jumpf, jumpb, jumpn
  //jumping
  if ((nextAction = CheckForJumping(rawInput.Latest(), context))) return nextAction;

  if (HasState(rawInput.Latest(), InputState::DOWN))
  {
    return new StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::NONE>("Crouching", facingRight, Vector2<float>(0, 0));
  }

  if ((nextAction = CheckForDash(rawInput, context))) return nextAction;

  std::string walkAnimLeft = !facingRight ? "WalkF" : "WalkB";
  std::string walkAnimRight = !facingRight ? "WalkB" : "WalkF";

  if (HasState(rawInput.Latest(), InputState::LEFT))
    return new LoopedAction<StanceState::STANDING, ActionState::NONE>(walkAnimLeft, facingRight, Vector2<float>(-0.5f * ActionParams::baseWalkSpeed, 0));
  else if (HasState(rawInput.Latest(), InputState::RIGHT))
    return new LoopedAction<StanceState::STANDING, ActionState::NONE>(walkAnimRight, facingRight, Vector2<float>(0.5f * ActionParams::baseWalkSpeed, 0));

  // Stopped
  return new LoopedAction<StanceState::STANDING, ActionState::NONE>("Idle", facingRight, Vector2<float>::Zero);
}

//______________________________________________________________________________
template <> IAction* LoopedAction<StanceState::JUMPING, ActionState::NONE>::HandleInput(const InputBuffer& rawInput, const StateComponent& context)
{
  // cannot block while jumping (for now)
  // if get hit while jumping leads to knockdown?
  IAction* onHitAction = CheckHits(rawInput.Latest(), context, false, true);
  if (onHitAction) return onHitAction;

  if (HasState(context.collision, CollisionSide::DOWN))
  {
    // when going back to neutral, change facing
    OnActionComplete();
    return new LoopedAction<StanceState::STANDING, ActionState::NONE>("Idle", context.onLeftSide, Vector2<float>(0, 0));
  }

  return GetAttacksFromNeutral<StanceState::JUMPING>(rawInput, context);
}

//______________________________________________________________________________
template <> IAction* LoopedAction<StanceState::CROUCHING, ActionState::NONE>::HandleInput(const InputBuffer& rawInput, const StateComponent& context)
{
  bool facingRight = context.onLeftSide;

  IAction* attackAction = GetAttacksFromNeutral<StanceState::CROUCHING>(rawInput, context);
  if (attackAction) return attackAction;

  IAction* onHitAction = CheckHits(rawInput.Latest(), context, true, false);
  if (onHitAction) return onHitAction;

  if (context.collision == CollisionSide::NONE)
  {
    OnActionComplete();
    return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", facingRight);
  }

  //if you arent attacking, you can move forward, move backward, crouch, stand, jumpf, jumpb, jumpn
  //jumping


  if (HasState(rawInput.Latest(), InputState::UP))
  {
    if (HasState(rawInput.Latest(), InputState::LEFT))
      return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", facingRight, Vector2<float>(-0.5f * ActionParams::baseWalkSpeed, -UniversalPhysicsSettings::Get().JumpVelocity));
    else if (HasState(rawInput.Latest(), InputState::RIGHT))
      return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", facingRight, Vector2<float>(0.5f * ActionParams::baseWalkSpeed, -UniversalPhysicsSettings::Get().JumpVelocity));
    return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", facingRight, Vector2<float>(0.0f, -UniversalPhysicsSettings::Get().JumpVelocity));
  }

  if (HasState(rawInput.Latest(), InputState::DOWN))
  {
    return new LoopedAction<StanceState::CROUCHING, ActionState::NONE>("Crouch", facingRight, Vector2<float>(0.0, 0.0));
  }

  std::string walkAnimLeft = !facingRight ? "WalkF" : "WalkB";
  std::string walkAnimRight = !facingRight ? "WalkB" : "WalkF";

  if (HasState(rawInput.Latest(), InputState::LEFT))
    return new LoopedAction<StanceState::STANDING, ActionState::NONE>(walkAnimLeft, facingRight, Vector2<float>(-0.5f * ActionParams::baseWalkSpeed, 0));
  else if (HasState(rawInput.Latest(), InputState::RIGHT))
    return new LoopedAction<StanceState::STANDING, ActionState::NONE>(walkAnimRight, facingRight, Vector2<float>(0.5f * ActionParams::baseWalkSpeed, 0));

  // state hasn't changed
  return new LoopedAction<StanceState::STANDING, ActionState::NONE>("Idle", facingRight, Vector2<float>(0.0, 0.0));
}

//______________________________________________________________________________
template <> IAction* StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::NONE>::HandleInput(const InputBuffer& rawInput, const StateComponent& context)
{
  bool facingRight = context.onLeftSide;

  IAction* onHitAction = CheckHits(rawInput.Latest(), context, true, false);
  if (onHitAction) return onHitAction;

  if (context.collision == CollisionSide::NONE)
  {
    OnActionComplete();
    return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", facingRight);
  }

  if (AnimatedAction<StanceState::CROUCHING, ActionState::NONE>::_complete)
  {
    return GetFollowUpAction(rawInput, context);
  }

  IAction* attackAction = GetAttacksFromNeutral<StanceState::CROUCHING>(rawInput, context);
  if (attackAction) return attackAction;

  //if you arent attacking, you can move forward, move backward, crouch, stand, jumpf, jumpb, jumpn
  //jumping


  if (HasState(rawInput.Latest(), InputState::UP))
  {
    if (HasState(rawInput.Latest(), InputState::LEFT))
      return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", facingRight, Vector2<float>(-0.5f * ActionParams::baseWalkSpeed, -UniversalPhysicsSettings::Get().JumpVelocity));
    else if (HasState(rawInput.Latest(), InputState::RIGHT))
      return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", facingRight, Vector2<float>(0.5f * ActionParams::baseWalkSpeed, -UniversalPhysicsSettings::Get().JumpVelocity));
    return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", facingRight, Vector2<float>(0.0f, -UniversalPhysicsSettings::Get().JumpVelocity));
  }

  if (HasState(rawInput.Latest(), InputState::DOWN))
  {
    return nullptr;
  }

  std::string walkAnimLeft = !facingRight ? "WalkF" : "WalkB";
  std::string walkAnimRight = !facingRight ? "WalkB" : "WalkF";

  if (HasState(rawInput.Latest(), InputState::LEFT))
    return new LoopedAction<StanceState::STANDING, ActionState::NONE>(walkAnimLeft, facingRight, Vector2<float>(-0.5f * ActionParams::baseWalkSpeed, 0));
  else if (HasState(rawInput.Latest(), InputState::RIGHT))
    return new LoopedAction<StanceState::STANDING, ActionState::NONE>(walkAnimRight, facingRight, Vector2<float>(0.5f * ActionParams::baseWalkSpeed, 0));

  // if not holding down, brought back to standing
  return new LoopedAction<StanceState::STANDING, ActionState::NONE>("Idle", facingRight, Vector2<float>(0.0, 0.0));
}

#ifndef _WIN32
//______________________________________________________________________________
template <> IAction* StateLockedAnimatedAction<StanceState::KNOCKDOWN, ActionState::HITSTUN>::HandleInput(const InputBuffer& rawInput, const StateComponent& context)
{
  if (_complete) return new StateLockedAnimatedAction<StanceState::KNOCKDOWN, ActionState::NONE>("Knockdown_OnGround", context.onLeftSide);
  return nullptr;
}

//______________________________________________________________________________
template <> IAction* StateLockedAnimatedAction<StanceState::KNOCKDOWN, ActionState::NONE>::HandleInput(const InputBuffer& rawInput, const StateComponent& context)
{
  if (rawInput.Latest() != InputState::NONE || _complete)
    return new LoopedAction<StanceState::STANDING, ActionState::NONE>("Idle", context.onLeftSide);
  return nullptr;
}
#endif
