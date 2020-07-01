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
template <> IAction* GetAttacksFromNeutral<StanceState::STANDING>(const InputBuffer& rawInput, bool facingRight)
{
  if (HasState(rawInput.Latest(), InputState::BTN1) || HasState(rawInput.Latest(), InputState::BTN2) || HasState(rawInput.Latest(), InputState::BTN3))
  {
    //!!!! TESTING SPECIAL MOVES HERE
    bool donkeyKick = (rawInput.Evaluate(UnivSpecMoveDict) == SpecialInputState::QCF && facingRight) || (rawInput.Evaluate(UnivSpecMoveDict) == SpecialInputState::QCB && !facingRight);
    bool tatsu = (rawInput.Evaluate(UnivSpecMoveDict) == SpecialInputState::QCF && !facingRight) || (rawInput.Evaluate(UnivSpecMoveDict) == SpecialInputState::QCB && facingRight);
    bool dp = (rawInput.Evaluate(UnivSpecMoveDict) == SpecialInputState::DPF && facingRight) || (rawInput.Evaluate(UnivSpecMoveDict) == SpecialInputState::DPB && !facingRight);
    if (donkeyKick)
      return new SpecialMoveAttack<StanceState::STANDING, ActionState::NONE>("SpecialMove3", facingRight);
    else if(tatsu)
      return new SpecialMoveAttack<StanceState::STANDING, ActionState::NONE>("SpecialMove4", facingRight);
    else if (dp)
      return new SpecialMoveAttack<StanceState::STANDING, ActionState::NONE>("SpecialMove2", facingRight);
  }

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
template <> IAction* GetAttacksFromNeutral<StanceState::CROUCHING>(const InputBuffer& rawInput, bool facingRight)
{
  return GetAttacksFromNeutral<StanceState::STANDING>(rawInput, facingRight);
}

//______________________________________________________________________________
template <> IAction* GetAttacksFromNeutral<StanceState::JUMPING>(const InputBuffer& rawInput, bool facingRight)
{
  // prioritize attacks
  // when attacking in the air, facing direction is not changed
  if (HasState(rawInput.Latest(), InputState::BTN1))
  {
    return new AttackAction<StanceState::JUMPING, ActionState::LIGHT>("JumpingLight", facingRight);
  }

  else if (HasState(rawInput.Latest(), InputState::BTN2))
  {
    return new AttackAction<StanceState::JUMPING, ActionState::MEDIUM>("JumpingMedium", facingRight);
  }

  else if (HasState(rawInput.Latest(), InputState::BTN3))
  {
    return new AttackAction<StanceState::JUMPING, ActionState::HEAVY>("JumpingHeavy", facingRight);
  }

  // state hasn't changed
  return nullptr;
}

//______________________________________________________________________________
IAction* CheckHits(const InputState& rawInput, const StateComponent& context)
{
  bool facingRight = context.onLeftSide;
  if (context.hitThisFrame)
  {
    bool blockedRight = HasState(rawInput, InputState::LEFT) && !context.hitOnLeftSide;
    bool blockedLeft = HasState(rawInput, InputState::RIGHT) && context.hitOnLeftSide;
    int neutralFrame = context.frameData.active + context.frameData.recover + 1;
    if (blockedRight || blockedLeft)
      return new OnRecvHitAction<StanceState::STANDING, ActionState::BLOCKSTUN>("Block", facingRight, neutralFrame + context.frameData.onBlockAdvantage, Vector2<float>::Zero);
    else
      return new OnRecvHitAction<StanceState::STANDING, ActionState::HITSTUN>("HeavyHitstun", facingRight, neutralFrame + context.frameData.onHitAdvantage, context.frameData.knockback, context.frameData.damage);
  }
  return nullptr;
}

//______________________________________________________________________________
IAction* CheckForSpecialCancel(const InputBuffer& rawInput, const StateComponent& context)
{
  //!!!! TESTING SPECIAL MOVE CANCELS HERE
  if (context.hitting)
  {
    if (HasState(rawInput.Latest(), InputState::BTN1) || HasState(rawInput.Latest(), InputState::BTN2) || HasState(rawInput.Latest(), InputState::BTN3))
    {
      bool qcf = rawInput.Evaluate(UnivSpecMoveDict) == SpecialInputState::QCF && context.onLeftSide;
      bool qcb = rawInput.Evaluate(UnivSpecMoveDict) == SpecialInputState::QCB && !context.onLeftSide;
      bool dpf = rawInput.Evaluate(UnivSpecMoveDict) == SpecialInputState::DPF && context.onLeftSide;
      bool dpb = rawInput.Evaluate(UnivSpecMoveDict) == SpecialInputState::DPB && !context.onLeftSide;
      if (qcf || qcb)
        return new SpecialMoveAttack<StanceState::STANDING, ActionState::NONE>("SpecialMove3", context.onLeftSide);
      else if (dpf || dpb)
        return new SpecialMoveAttack<StanceState::STANDING, ActionState::NONE>("SpecialMove2", context.onLeftSide);
    }
  }
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
    int neutralFrame = context.frameData.active + context.frameData.recover + 1;
    return new OnRecvHitAction<StanceState::STANDING, ActionState::HITSTUN>("HeavyHitstun", context.onLeftSide, neutralFrame + context.frameData.onHitAdvantage, context.frameData.knockback);
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
