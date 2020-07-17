#include "StateMachine/ActionUtil.h"
#include "StateMachine/AttackAction.h"
#include "StateMachine/TimedAction.h"

//______________________________________________________________________________
float ActionParams::baseWalkSpeed = 300.0f * 1.5f;
int ActionParams::nDashFrames = 23;

//______________________________________________________________________________
float Interpolation::Plateau::a = 2.0f;
float Interpolation::Plateau::modifier = 1.75f;
float Interpolation::Plateau::d = 0.001f;
float Interpolation::Plateau::xAxisOffset = 10.0f;

//______________________________________________________________________________
float Interpolation::Plateau::F(float x, float xMax, float yMax)
{
  x = x + xAxisOffset;
  const float pi = 3.14159265358979323846f;
  const float k = (a / pi) * std::sinf(pi / (2.0f * a));

  float scaledXValue = 0.5f * (x / xMax) - 0.5f;
  float x2a = std::powf(scaledXValue, 2.0f * a);
  return ((d * k) / (d + x2a)) * (modifier * yMax);
}

//______________________________________________________________________________
template <> IAction* GetAttacksFromNeutral<StanceState::STANDING>(const InputBuffer& rawInput, const StateComponent& context)
{
  if (auto specialMove = CheckSpecials(rawInput, context))
    return specialMove;

  const bool& facingRight = context.onLeftSide;
  // prioritize attacks
  if (HasState(rawInput.Latest(), InputState::BTN1))
  {
    if (HasState(rawInput.Latest(), InputState::DOWN))
      return new GroundedStaticAttack<StanceState::CROUCHING, ActionState::LIGHT>("CrouchingLight", facingRight);
    else
      return new GroundedStaticAttack<StanceState::STANDING, ActionState::LIGHT>("StandingLight", facingRight);
  }

  else if (HasState(rawInput.Latest(), InputState::BTN2))
  {
    if (HasState(rawInput.Latest(), InputState::DOWN))
      return new GroundedStaticAttack<StanceState::CROUCHING, ActionState::MEDIUM>("CrouchingMedium", facingRight);
    else
      return new GroundedStaticAttack<StanceState::STANDING, ActionState::MEDIUM>("StandingMedium", facingRight);
  }

  else if (HasState(rawInput.Latest(), InputState::BTN3))
  {
    if (HasState(rawInput.Latest(), InputState::DOWN))
      return new GroundedStaticAttack<StanceState::CROUCHING, ActionState::HEAVY>("CrouchingHeavy", facingRight);
    else
      return new GroundedStaticAttack<StanceState::STANDING, ActionState::HEAVY>("StandingHeavy", facingRight);
  }
  return nullptr;
}

//______________________________________________________________________________
template <> IAction* GetAttacksFromNeutral<StanceState::CROUCHING>(const InputBuffer& rawInput, const StateComponent& context)
{
  return GetAttacksFromNeutral<StanceState::STANDING>(rawInput, context);
}

//______________________________________________________________________________
template <> IAction* GetAttacksFromNeutral<StanceState::JUMPING>(const InputBuffer& rawInput, const StateComponent& context)
{
  // prioritize attacks
  // when attacking in the air, facing direction is not changed
  if (HasState(rawInput.Latest(), InputState::BTN1))
  {
    return new AttackAction<StanceState::JUMPING, ActionState::LIGHT>("JumpingLight", context.onLeftSide);
  }

  else if (HasState(rawInput.Latest(), InputState::BTN2))
  {
    return new AttackAction<StanceState::JUMPING, ActionState::MEDIUM>("JumpingMedium", context.onLeftSide);
  }

  else if (HasState(rawInput.Latest(), InputState::BTN3))
  {
    return new AttackAction<StanceState::JUMPING, ActionState::HEAVY>("JumpingHeavy", context.onLeftSide);
  }

  // state hasn't changed
  return nullptr;
}

//______________________________________________________________________________
IAction* CheckSpecials(const InputBuffer& rawInput, const StateComponent& context)
{
  const bool& facingRight = context.onLeftSide;
  if (HasState(rawInput.Latest(), InputState::BTN1) || HasState(rawInput.Latest(), InputState::BTN2) || HasState(rawInput.Latest(), InputState::BTN3))
  {
    //!!!! TESTING SPECIAL MOVES HERE
    bool fireball = (rawInput.Evaluate(UnivSpecMoveDict) == SpecialInputState::QCF && facingRight) || (rawInput.Evaluate(UnivSpecMoveDict) == SpecialInputState::QCB && !facingRight);
    bool donkeyKick = fireball && (HasState(rawInput.Latest(), InputState::BTN3));
    bool tatsu = (rawInput.Evaluate(UnivSpecMoveDict) == SpecialInputState::QCF && !facingRight) || (rawInput.Evaluate(UnivSpecMoveDict) == SpecialInputState::QCB && facingRight);
    bool dp = (rawInput.Evaluate(UnivSpecMoveDict) == SpecialInputState::DPF && facingRight) || (rawInput.Evaluate(UnivSpecMoveDict) == SpecialInputState::DPB && !facingRight);
    if (donkeyKick)
      return new SpecialMoveAttack<StanceState::STANDING, ActionState::NONE>("SpecialMove3", facingRight);
    else if (fireball)
      return new SpecialMoveAttack<StanceState::STANDING, ActionState::NONE>("SpecialMove1", facingRight);
    else if (tatsu)
      return new SpecialMoveAttack<StanceState::STANDING, ActionState::NONE>("SpecialMove4", facingRight);
    else if (dp)
      return new SpecialMoveAttack<StanceState::STANDING, ActionState::NONE>("SpecialMove2", facingRight);
  }
  return nullptr;
}


//______________________________________________________________________________
IAction* CheckHits(const InputState& rawInput, const StateComponent& context)
{
  bool facingRight = context.onLeftSide;
  if (context.hitThisFrame)
  {
    bool blockedRight = HasState(rawInput, InputState::LEFT) && context.onLeftSide;
    bool blockedLeft = HasState(rawInput, InputState::RIGHT) && !context.onLeftSide;
    if (blockedRight || blockedLeft)
      return new OnRecvHitAction<StanceState::STANDING, ActionState::BLOCKSTUN>("Block", facingRight, context.hitData.framesInStunBlock, Vector2<float>::Zero);
    else
      return new OnRecvHitAction<StanceState::STANDING, ActionState::HITSTUN>("HeavyHitstun", facingRight, context.hitData.framesInStunHit, context.hitData.knockback, context.hitData.damage);
  }
  return nullptr;
}

//______________________________________________________________________________
IAction* StateLockedHandleInput(const InputBuffer& rawInput, const StateComponent& context, IAction* action, bool actionComplete)
{
  if (actionComplete)
  {
    return action->GetFollowUpAction(rawInput, context);
  }

  if (context.hitThisFrame)
  {
    return new OnRecvHitAction<StanceState::STANDING, ActionState::HITSTUN>("HeavyHitstun", context.onLeftSide, context.hitData.framesInStunHit, context.hitData.knockback);
  }

  if (action->GetStance() == StanceState::JUMPING)
  {
    if (HasState(context.collision, CollisionSide::DOWN))
    {
      return action->GetFollowUpAction(rawInput, context);
    }
  }
  return nullptr;
}
