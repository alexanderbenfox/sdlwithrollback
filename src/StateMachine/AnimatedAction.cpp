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

  if (context.collision == CollisionSide::NONE)
    return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Falling", facingRight);

  // process attacks before hits so that if you press a button while attacked, you still get attacked
  IAction* attackAction = GetAttacksFromNeutral<StanceState::STANDING>(rawInput, context);
  if (attackAction) return attackAction;

  IAction* onHitAction = CheckHits(rawInput.Latest(), context);
  if (onHitAction) return onHitAction;

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
    return new StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::NONE>("Crouching", facingRight, Vector2<float>(0, 0));
  }

  // check dashing
  std::string dashAnimLeft = !facingRight ? "ForwardDash" : "BackDash";
  std::string dashAnimRight = !facingRight ? "BackDash" : "ForwardDash";
  if (HasState(rawInput.Latest(), InputState::LEFT))
  {
    if (HasState(rawInput.Latest(), InputState::BTN4) || rawInput.Evaluate(UnivSpecMoveDict) == SpecialInputState::LDash)
    {
      return new DashAction(dashAnimLeft, facingRight, ActionParams::nDashFrames, -1.5f * ActionParams::baseWalkSpeed);
    }
  }
  else if (HasState(rawInput.Latest(), InputState::RIGHT))
  {
    if(HasState(rawInput.Latest(), InputState::BTN4) || rawInput.Evaluate(UnivSpecMoveDict) == SpecialInputState::RDash)
    {
      return new DashAction(dashAnimRight, facingRight, ActionParams::nDashFrames, 1.5f * ActionParams::baseWalkSpeed);
    }
  }

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
  IAction* onHitAction = CheckHits(rawInput.Latest(), context);
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
  if (context.collision == CollisionSide::NONE)
  {
    OnActionComplete();
    return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", facingRight);
  }

  IAction* attackAction = GetAttacksFromNeutral<StanceState::CROUCHING>(rawInput, context);
  if (attackAction) return attackAction;

  IAction* onHitAction = CheckHits(rawInput.Latest(), context);
  if (onHitAction) return onHitAction;

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
  if (context.collision == CollisionSide::NONE)
  {
    OnActionComplete();
    return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", facingRight);
  }

  IAction* onHitAction = CheckHits(rawInput.Latest(), context);
  if (onHitAction) return onHitAction;

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
