#include "Actions.h"
#include "Entity.h"

#include "Components/Sprite.h"
#include "Components/Physics.h"
#include "Components/GameActor.h"

const float _baseSpeed = 300.0f * 1.5f;

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
AnimatedAction<Stance, Action>::AnimatedAction(const std::string& animation, Entity* actor) : _animation(animation)
{
  if (auto animator = actor->GetComponent<Animator>())
  {
    if (animator->GetAnimationByName(animation))
    {
      animator->Play(animation, true);
    }
    else
    {
      if (auto ac = actor->GetComponent<GameActor>())
        ac->OnActionComplete(this);
    }
  }
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
AnimatedAction<Stance, Action>::AnimatedAction(const std::string& animation, Entity* actor, Vector2<float> instVeclocity) : AnimatedAction<Stance, Action>(animation, actor)
{
  if (auto mover = actor->GetComponent<Physics>())
  {
    mover->ApplyVelocity(instVeclocity);
  }
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
StateLockedAnimatedAction<Stance, Action>::StateLockedAnimatedAction(const std::string& animation, Entity* actor) : AnimatedAction<Stance, Action>(animation, actor)
{
  if (auto animator = actor->GetComponent<Animator>())
  {
    if (animator->GetAnimationByName(animation))
    {
      animator->Play(animation, false);
    }
    else
    {
      if (auto ac = actor->GetComponent<GameActor>())
        ac->OnActionComplete(this);
    }
  }
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
void StateLockedAnimatedAction<Stance, Action>::OnUpdate(Entity* actor)
{
  // non-moving command so just make sure you arent moving
  if (Stance != StanceState::JUMPING)
  {
    if (actor->GetComponent<Physics>())
    {
      actor->GetComponent<Physics>()->ChangeXVelocity(0);
    }
  }

  // is polling or registering and unregistering listener function better??
  if (auto animator = actor->GetComponent<Animator>())
  {
    int currentAnimationFrame = animator->GetAnimationByName(this->_animation)->GetFrameCount() - 1;

    if (currentAnimationFrame == animator->GetShowingFrame())
    {
      IAction* nextAction = OnActionComplete(actor);
      if (nextAction == this)
        return;

      if (auto ac = actor->GetComponent<GameActor>())
      {
        ac->OnActionComplete(this);
        if (nextAction)
          ac->BeginNewAction(nextAction);
      }
    }
  }
}

//______________________________________________________________________________
template <> IAction* LoopedAction<StanceState::STANDING, ActionState::NONE>::HandleInput(InputState input, CollisionSide collision, Entity* actor)
{
  if (collision == CollisionSide::NONE)
  {
    return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", actor);
  }

  // prioritize attacks
  if (HasState(input, InputState::BTN1))
  {
    if (HasState(input, InputState::DOWN))
      return new StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::LIGHT>("CrouchingLight", actor);
    else
      return new StateLockedAnimatedAction<StanceState::STANDING, ActionState::LIGHT>("StandingLight", actor);
  }

  else if (HasState(input, InputState::BTN2))
  {
    if (HasState(input, InputState::DOWN))
      return new StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::MEDIUM>("CrouchingMedium", actor);
    else
      return new StateLockedAnimatedAction<StanceState::STANDING, ActionState::MEDIUM>("StandingMedium", actor);
  }

  else if (HasState(input, InputState::BTN3))
  {
    if (HasState(input, InputState::DOWN))
      return new StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::HEAVY>("CrouchingHeavy", actor);
    else
      return new StateLockedAnimatedAction<StanceState::STANDING, ActionState::HEAVY>("StandingHeavy", actor);
  }

  //if you arent attacking, you can move forward, move backward, crouch, stand, jumpf, jumpb, jumpn
  //jumping

  if (HasState(input, InputState::UP))
  {
    if (HasState(input, InputState::LEFT))
      return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", actor, Vector2<float>(-0.5f * _baseSpeed, -UniversalPhysicsSettings::Get().JumpVelocity));
    else if (HasState(input, InputState::RIGHT))
      return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", actor, Vector2<float>(0.5f * _baseSpeed, -UniversalPhysicsSettings::Get().JumpVelocity));
    return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", actor, Vector2<float>(0.0f, -UniversalPhysicsSettings::Get().JumpVelocity));
  }
  if (HasState(input, InputState::DOWN))
  {
    return new StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::NONE>("Crouching", actor);
  }
  if (HasState(input, InputState::LEFT))
    return new LoopedAction<StanceState::STANDING, ActionState::NONE>("WalkB", actor, Vector2<float>(-0.5f * _baseSpeed, 0));
  else if (HasState(input, InputState::RIGHT))
    return new LoopedAction<StanceState::STANDING, ActionState::NONE>("WalkF", actor, Vector2<float>(0.5f * _baseSpeed, 0));

  // Stopped
  return new LoopedAction<StanceState::STANDING, ActionState::NONE>("Idle", actor, Vector2<float>::Zero());
}

//______________________________________________________________________________
template <> IAction* LoopedAction<StanceState::JUMPING, ActionState::NONE>::HandleInput(InputState input, CollisionSide collision, Entity* actor)
{
  if (HasState(collision, CollisionSide::DOWN))
  {
    return new LoopedAction<StanceState::STANDING, ActionState::NONE>("Idle", actor, Vector2<float>(0,0));
  }

  // prioritize attacks
  if (HasState(input, InputState::BTN1))
  {
    return new StateLockedAnimatedAction<StanceState::JUMPING, ActionState::LIGHT>("JumpingLight", actor);
  }

  else if (HasState(input, InputState::BTN2))
  {
    return new StateLockedAnimatedAction<StanceState::JUMPING, ActionState::MEDIUM>("JumpingMedium", actor);
  }

  else if (HasState(input, InputState::BTN3))
  {
    return new StateLockedAnimatedAction<StanceState::JUMPING, ActionState::HEAVY>("JumpingHeavy", actor);
  }

  // state hasn't changed
  return nullptr;
}

//______________________________________________________________________________
template <> IAction* LoopedAction<StanceState::CROUCHING, ActionState::NONE>::HandleInput(InputState input, CollisionSide collision, Entity* actor)
{
  if (collision == CollisionSide::NONE)
  {
    return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", actor);
  }

  // prioritize attacks
  if (HasState(input, InputState::BTN1))
  {
    if (HasState(input, InputState::DOWN))
      return new StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::LIGHT>("CrouchingLight", actor);
    else
      return new StateLockedAnimatedAction<StanceState::STANDING, ActionState::LIGHT>("StandingLight", actor);
  }

  else if (HasState(input, InputState::BTN2))
  {
    if (HasState(input, InputState::DOWN))
      return new StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::MEDIUM>("CrouchingMedium", actor);
    else
      return new StateLockedAnimatedAction<StanceState::STANDING, ActionState::MEDIUM>("StandingMedium", actor);
  }

  else if (HasState(input, InputState::BTN3))
  {
    if (HasState(input, InputState::DOWN))
      return new StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::HEAVY>("CrouchingHeavy", actor);
    else
      return new StateLockedAnimatedAction<StanceState::STANDING, ActionState::HEAVY>("StandingHeavy", actor);
  }

  //if you arent attacking, you can move forward, move backward, crouch, stand, jumpf, jumpb, jumpn
  //jumping
  

  if (HasState(input, InputState::UP))
  {
    if (HasState(input, InputState::LEFT))
      return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", actor, Vector2<float>(-0.5f * _baseSpeed, -UniversalPhysicsSettings::Get().JumpVelocity));
    else if (HasState(input, InputState::RIGHT))
      return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", actor, Vector2<float>(0.5f * _baseSpeed, -UniversalPhysicsSettings::Get().JumpVelocity));
    return new LoopedAction<StanceState::JUMPING, ActionState::NONE>("Jumping", actor, Vector2<float>(0.0f, -UniversalPhysicsSettings::Get().JumpVelocity));
  }

  if (HasState(input, InputState::DOWN))
  {
    return new LoopedAction<StanceState::CROUCHING, ActionState::NONE>("Crouch", actor, Vector2<float>(0.0, 0.0));
  }

  if (HasState(input, InputState::LEFT))
    return new LoopedAction<StanceState::STANDING, ActionState::NONE>("WalkB", actor, Vector2<float>(-0.5f * _baseSpeed, 0));
  else if (HasState(input, InputState::RIGHT))
    return new LoopedAction<StanceState::STANDING, ActionState::NONE>("WalkF", actor, Vector2<float>(0.5f * _baseSpeed, 0));

  // state hasn't changed
  return new LoopedAction<StanceState::STANDING, ActionState::NONE>("Idle", actor, Vector2<float>(0.0, 0.0));
}

//______________________________________________________________________________
template <> void LoopedAction<StanceState::JUMPING, ActionState::NONE>::OnUpdate(Entity* actor)
{
  if (auto mover = actor->GetComponent<Physics>())
  {
    if (auto animator = actor->GetComponent<Animator>())
    {
      if (mover->GetVelocity().y < 0)
      {
        animator->Play("Jumping", false);
      }
      else
      {
        animator->Play("Falling", false);
      }
    }
  }
}
