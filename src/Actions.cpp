#include "Actions.h"
#include "Entity.h"
#include "GameState/Player.h"

#include "Components/Animator.h"
#include "Components/Physics.h"
#include "Components/GameActor.h"

const float _baseSpeed = 300.0f * 1.5f;

template <> IAction* GetAttacksFromNeutral<StanceState::STANDING>(const InputState& rawInput, bool facingRight)
{
  // prioritize attacks
  if (HasState(rawInput, InputState::BTN1))
  {
    if (HasState(rawInput, InputState::DOWN))
      return new GroundedStaticAttack<StanceState::CROUCHING, ActionState::LIGHT>("CrouchingLight", facingRight);
    else
      return new GroundedStaticAttack<StanceState::STANDING, ActionState::LIGHT>("StandingLight", facingRight);
  }

  else if (HasState(rawInput, InputState::BTN2))
  {
    if (HasState(rawInput, InputState::DOWN))
      return new GroundedStaticAttack<StanceState::CROUCHING, ActionState::MEDIUM>("CrouchingMedium", facingRight);
    else
      return new GroundedStaticAttack<StanceState::STANDING, ActionState::MEDIUM>("StandingMedium", facingRight);
  }

  else if (HasState(rawInput, InputState::BTN3))
  {
    if (HasState(rawInput, InputState::DOWN))
      return new GroundedStaticAttack<StanceState::CROUCHING, ActionState::HEAVY>("CrouchingHeavy", facingRight);
    else
      return new GroundedStaticAttack<StanceState::STANDING, ActionState::HEAVY>("StandingHeavy", facingRight);
  }
  return nullptr;
}

template <> IAction* GetAttacksFromNeutral<StanceState::CROUCHING>(const InputState& rawInput, bool facingRight)
{
  return GetAttacksFromNeutral<StanceState::STANDING>(rawInput, facingRight);
}

template <> IAction* GetAttacksFromNeutral<StanceState::JUMPING>(const InputState& rawInput, bool facingRight)
{
  // prioritize attacks
  // when attacking in the air, facing direction is not changed
  if (HasState(rawInput, InputState::BTN1))
  {
    return new StateLockedAnimatedAction<StanceState::JUMPING, ActionState::LIGHT>("JumpingLight", facingRight);
  }

  else if (HasState(rawInput, InputState::BTN2))
  {
    return new StateLockedAnimatedAction<StanceState::JUMPING, ActionState::MEDIUM>("JumpingMedium", facingRight);
  }

  else if (HasState(rawInput, InputState::BTN3))
  {
    return new StateLockedAnimatedAction<StanceState::JUMPING, ActionState::HEAVY>("JumpingHeavy", facingRight);
  }

  // state hasn't changed
  return nullptr;
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
void AnimatedAction<Stance, Action>::Enact(Entity* actor)
{
  if (auto animator = actor->GetComponent<Animator>())
  {
    animator->ChangeListener(this);
    if (animator->GetAnimationByName(_animation))
    {
      animator->Play(_animation, _loopedAnimation, !_facingRight);
    }
    else
    {
      if (auto ac = actor->GetComponent<GameActor>())
        ac->OnActionComplete(this);
    }
  }

  if (_movementType)
  {
    if (auto mover = actor->GetComponent<Physics>())
      mover->_vel = _velocity;
      //mover->ApplyVelocity(_velocity);
  }
}

//______________________________________________________________________________
template <> IAction* StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::NONE>::GetFollowUpAction()
{
  return new LoopedAction<StanceState::CROUCHING, ActionState::NONE>("Crouch", _facingRight);
}

//______________________________________________________________________________
template <> IAction* LoopedAction<StanceState::STANDING, ActionState::NONE>::HandleInput(const InputState& rawInput, const GameContext& context)
{
  bool facingRight = context.onLeftSide;

  if (context.collision == CollisionSide::NONE)
  {
    if(context.movement.y < 0)
      return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", facingRight);
    else
      return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Falling", facingRight);
  }

  IAction* attackAction = GetAttacksFromNeutral<StanceState::STANDING>(rawInput, context.onLeftSide);
  if(attackAction) return attackAction;

  //if you arent attacking, you can move forward, move backward, crouch, stand, jumpf, jumpb, jumpn
  //jumping

  if (HasState(rawInput, InputState::UP))
  {
    if (HasState(rawInput, InputState::LEFT))
      return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", facingRight, Vector2<float>(-0.5f * _baseSpeed, -UniversalPhysicsSettings::Get().JumpVelocity));
    else if (HasState(rawInput, InputState::RIGHT))
      return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", facingRight, Vector2<float>(0.5f * _baseSpeed, -UniversalPhysicsSettings::Get().JumpVelocity));
    return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", facingRight, Vector2<float>(0.0f, -UniversalPhysicsSettings::Get().JumpVelocity));
  }

  if (HasState(rawInput, InputState::DOWN))
  {
    return new StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::NONE>("Crouching", facingRight);
  }

  std::string walkAnimLeft = !facingRight ? "WalkF" : "WalkB";
  std::string walkAnimRight = !facingRight ? "WalkB" : "WalkF";

  if (HasState(rawInput, InputState::LEFT))
    return new LoopedAction<StanceState::STANDING, ActionState::NONE>(walkAnimLeft, facingRight, Vector2<float>(-0.5f * _baseSpeed, 0));
  else if (HasState(rawInput, InputState::RIGHT))
    return new LoopedAction<StanceState::STANDING, ActionState::NONE>(walkAnimRight, facingRight, Vector2<float>(0.5f * _baseSpeed, 0));

  // Stopped
  return new LoopedAction<StanceState::STANDING, ActionState::NONE>("Idle", facingRight, Vector2<float>::Zero());
}

//______________________________________________________________________________
template <> IAction* LoopedAction<StanceState::JUMPING, ActionState::NONE>::HandleInput(const InputState& rawInput, const GameContext& context)
{
  if (HasState(context.collision, CollisionSide::DOWN))
  {
    // when going back to neutral, change facing
    return new LoopedAction<StanceState::STANDING, ActionState::NONE>("Idle", context.onLeftSide, Vector2<float>(0,0));
  }

  return GetAttacksFromNeutral<StanceState::JUMPING>(rawInput, _facingRight);
}

//______________________________________________________________________________
template <> IAction* LoopedAction<StanceState::CROUCHING, ActionState::NONE>::HandleInput(const InputState& rawInput, const GameContext& context)
{
  bool facingRight = context.onLeftSide;
  if (context.collision == CollisionSide::NONE)
  {
    return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", facingRight);
  }

  IAction* attackAction = GetAttacksFromNeutral<StanceState::CROUCHING>(rawInput, context.onLeftSide);
  if(attackAction) return attackAction;

  //if you arent attacking, you can move forward, move backward, crouch, stand, jumpf, jumpb, jumpn
  //jumping
  

  if (HasState(rawInput, InputState::UP))
  {
    if (HasState(rawInput, InputState::LEFT))
      return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", facingRight, Vector2<float>(-0.5f * _baseSpeed, -UniversalPhysicsSettings::Get().JumpVelocity));
    else if (HasState(rawInput, InputState::RIGHT))
      return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", facingRight, Vector2<float>(0.5f * _baseSpeed, -UniversalPhysicsSettings::Get().JumpVelocity));
    return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", facingRight, Vector2<float>(0.0f, -UniversalPhysicsSettings::Get().JumpVelocity));
  }

  if (HasState(rawInput, InputState::DOWN))
  {
    return new LoopedAction<StanceState::CROUCHING, ActionState::NONE>("Crouch", facingRight, Vector2<float>(0.0, 0.0));
  }

  std::string walkAnimLeft = !facingRight ? "WalkF" : "WalkB";
  std::string walkAnimRight = !facingRight ? "WalkB" : "WalkF";

  if (HasState(rawInput, InputState::LEFT))
    return new LoopedAction<StanceState::STANDING, ActionState::NONE>(walkAnimLeft, facingRight, Vector2<float>(-0.5f * _baseSpeed, 0));
  else if (HasState(rawInput, InputState::RIGHT))
    return new LoopedAction<StanceState::STANDING, ActionState::NONE>(walkAnimRight, facingRight, Vector2<float>(0.5f * _baseSpeed, 0));

  // state hasn't changed
  return new LoopedAction<StanceState::STANDING, ActionState::NONE>("Idle", facingRight, Vector2<float>(0.0, 0.0));
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
StateLockedAnimatedAction<Stance, Action>::StateLockedAnimatedAction(const std::string& animation, bool facingRight) : AnimatedAction<Stance, Action>(animation, facingRight)
{
  this->_loopedAnimation = false;
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
StateLockedAnimatedAction<Stance, Action>::StateLockedAnimatedAction(const std::string& animation, bool facingRight, Vector2<float> actionMovement) : AnimatedAction<Stance, Action>(animation, facingRight, actionMovement)
{
  this->_loopedAnimation = false;
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
IAction* StateLockedAnimatedAction<Stance, Action>::GetFollowUpAction()
{
  return new LoopedAction<Stance, ActionState::NONE>
    (Stance == StanceState::STANDING ? "Idle" : Stance == StanceState::CROUCHING ? "Crouch" : "Jumping", this->_facingRight);
}
