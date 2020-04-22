#include "Actions.h"
#include "Entity.h"

#include "Components/Animator.h"
#include "Components/Rigidbody.h"
#include "Components/GameActor.h"
#include "Components/AttackStateComponent.h"

const float _baseSpeed = 300.0f * 1.5f;

//______________________________________________________________________________
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

//______________________________________________________________________________
template <> IAction* GetAttacksFromNeutral<StanceState::CROUCHING>(const InputState& rawInput, bool facingRight)
{
  return GetAttacksFromNeutral<StanceState::STANDING>(rawInput, facingRight);
}

//______________________________________________________________________________
template <> IAction* GetAttacksFromNeutral<StanceState::JUMPING>(const InputState& rawInput, bool facingRight)
{
  // prioritize attacks
  // when attacking in the air, facing direction is not changed
  if (HasState(rawInput, InputState::BTN1))
  {
    return new AttackAction<StanceState::JUMPING, ActionState::LIGHT>("JumpingLight", facingRight);
  }

  else if (HasState(rawInput, InputState::BTN2))
  {
    return new AttackAction<StanceState::JUMPING, ActionState::MEDIUM>("JumpingMedium", facingRight);
  }

  else if (HasState(rawInput, InputState::BTN3))
  {
    return new AttackAction<StanceState::JUMPING, ActionState::HEAVY>("JumpingHeavy", facingRight);
  }

  // state hasn't changed
  return nullptr;
}

//______________________________________________________________________________
IAction* CheckHits(const InputState& rawInput, const GameContext& context)
{
  bool facingRight = context.onLeftSide;
  if (context.hitThisFrame)
  {
    bool blockedRight = HasState(rawInput, InputState::LEFT) && !context.hitOnLeftSide;
    bool blockedLeft = HasState(rawInput, InputState::RIGHT) && context.hitOnLeftSide;
    int neutralFrame = context.frameData.active + context.frameData.recover + 1;
    if (blockedRight || blockedLeft)
      return new OnRecvHitAction<StanceState::STANDING, ActionState::BLOCKSTUN>("Block", facingRight, neutralFrame + context.frameData.onBlockAdvantage, Vector2<float>::Zero());
    else
      return new OnRecvHitAction<StanceState::STANDING, ActionState::HITSTUN>("HeavyHitstun", facingRight, neutralFrame + context.frameData.onHitAdvantage, context.frameData.knockback);
  }
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

      // set the offset in the properties component (needs to be in a system)
      if(auto properties = actor->GetComponent<RenderProperties>())
      {
        properties->horizontalFlip = !_facingRight;
        properties->offset = animator->GetRenderOffset(!_facingRight);
      }

      if(auto renderer = actor->GetComponent<GraphicRenderer>())
      {
        // render from the sheet of the new animation
        renderer->SetRenderResource(animator->GetCurrentAnimation().GetSheetTexture());
        renderer->sourceRect = animator->GetCurrentAnimation().GetFrameSrcRect(0);
      }
    }
    else
    {
      if (auto ac = actor->GetComponent<GameActor>())
        ac->OnActionComplete(this);
    }
  }


  if (_movementType)
  {
    if (auto mover = actor->GetComponent<Rigidbody>())
      mover->_vel = _velocity;
      //mover->ApplyVelocity(_velocity);
  }

  if (_listener)
    _listener->SetStateInfo(Stance, Action);
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

  // process attacks before hits so that if you press a button while attacked, you still get attacked
  IAction* attackAction = GetAttacksFromNeutral<StanceState::STANDING>(rawInput, context.onLeftSide);
  if (attackAction) return attackAction;
  
  IAction* onHitAction = CheckHits(rawInput, context);
  if (onHitAction) return onHitAction;

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
    return new StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::NONE>("Crouching", facingRight, Vector2<float>(0,0));
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
  IAction* onHitAction = CheckHits(rawInput, context);
  if (onHitAction) return onHitAction;

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

  IAction* onHitAction = CheckHits(rawInput, context);
  if (onHitAction) return onHitAction;

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
TimedAction<Stance, Action>::~TimedAction<Stance, Action>()
{
  _actionTiming->Cancel();
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
void TimedAction<Stance, Action>::Enact(Entity* actor)
{
  AnimatedAction<Stance, Action>::Enact(actor);
  AnimatedAction<Stance, Action>::_complete = false;
  _actionTiming = std::make_shared<TimerComponent>([this]() { this->OnActionComplete(); }, _duration);
  actor->GetComponent<GameActor>()->timings.push_back(_actionTiming);
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
OnRecvHitAction<Stance, Action>::~OnRecvHitAction()
{
  // make sure this state component is removed
  ListenedAction::_listener->GetOwner()->RemoveComponent<HitStateComponent>();
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
void OnRecvHitAction<Stance, Action>::Enact(Entity* actor)
{
  TimedAction<Stance, Action>::Enact(actor);
  actor->AddComponent<HitStateComponent>();
  actor->GetComponent<HitStateComponent>()->SetTimer(TimedAction<Stance, Action>::_actionTiming.get());
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
IAction* OnRecvHitAction<Stance, Action>::GetFollowUpAction()
{
  return new LoopedAction<Stance, ActionState::NONE>
    (Stance == StanceState::STANDING ? "Idle" : Stance == StanceState::CROUCHING ? "Crouch" : "Jumping", this->_facingRight);
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
void OnRecvHitAction<Stance, Action>::OnActionComplete()
{
  ListenedAction::_listener->GetOwner()->RemoveComponent<HitStateComponent>();
  ListenedAction::OnActionComplete();
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

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
AttackAction<Stance, Action>::~AttackAction()
{
  // make sure this state component is removed
  ListenedAction::_listener->GetOwner()->RemoveComponent<AttackStateComponent>();
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
void AttackAction<Stance, Action>::Enact(Entity* actor)
{
  AnimatedAction<Stance, Action>::Enact(actor);
  if (auto animator = actor->GetComponent<Animator>())
  {
    if (Animation* anim = animator->GetAnimationByName(AnimatedAction<Stance, Action>::_animation))
    {
      actor->AddComponent<AttackStateComponent>();
      actor->GetComponent<AttackStateComponent>()->SetAnimation(anim);
    }
  }
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
void AttackAction<Stance, Action>::OnActionComplete()
{
  ListenedAction::_listener->GetOwner()->RemoveComponent<AttackStateComponent>();
  ListenedAction::OnActionComplete();
}

//______________________________________________________________________________
template <> IAction* StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::NONE>::HandleInput(const InputState& rawInput, const GameContext& context)
{

  bool facingRight = context.onLeftSide;
  if (context.collision == CollisionSide::NONE)
  {
    return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", facingRight);
  }

  IAction* attackAction = GetAttacksFromNeutral<StanceState::CROUCHING>(rawInput, context.onLeftSide);
  if(attackAction) return attackAction;

  IAction* onHitAction = CheckHits(rawInput, context);
  if (onHitAction) return onHitAction;

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
    return nullptr;
  }

  std::string walkAnimLeft = !facingRight ? "WalkF" : "WalkB";
  std::string walkAnimRight = !facingRight ? "WalkB" : "WalkF";

  if (HasState(rawInput, InputState::LEFT))
    return new LoopedAction<StanceState::STANDING, ActionState::NONE>(walkAnimLeft, facingRight, Vector2<float>(-0.5f * _baseSpeed, 0));
  else if (HasState(rawInput, InputState::RIGHT))
    return new LoopedAction<StanceState::STANDING, ActionState::NONE>(walkAnimRight, facingRight, Vector2<float>(0.5f * _baseSpeed, 0));

  if(AnimatedAction<StanceState::CROUCHING, ActionState::NONE>::_complete)
  {
    return GetFollowUpAction();
  }
  // state hasn't changed
  return new LoopedAction<StanceState::STANDING, ActionState::NONE>("Idle", facingRight, Vector2<float>(0.0, 0.0));
}
