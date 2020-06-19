#pragma once
#include "StateMachine/IAction.h"
#include "Components/Animator.h"

#include "Components/Rigidbody.h"
#include "Components/GameActor.h"
#include "Components/RenderComponent.h"
#include "Components/Hurtbox.h"

#include "GameManagement.h"

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
class AnimatedAction : public ListenedAction, public IAnimatorListener
{
public:
  //!
  AnimatedAction(const std::string& animation, bool facingRight) :
    _loopedAnimation(true), _animation(animation), _facingRight(facingRight), _movementType(false) {}
  //!
  AnimatedAction(const std::string& animation, bool facingRight, Vector2<float> instVelocity) :
    _loopedAnimation(true), _animation(animation), _facingRight(facingRight), _velocity(instVelocity), _movementType(true) {}

  //!
  virtual void Enact(Entity* actor) override;
  //!
  virtual IAction* HandleInput(const InputBuffer& rawInput, const StateComponent& context) override = 0;

  virtual void OnAnimationComplete(const std::string& completedAnimation) override;

  virtual void SetComplete() override { _complete = true; }

  virtual StanceState GetStance() override { return Stance; }
  virtual ActionState GetAction() override { return Action; }

protected:
  //!
  virtual IAction* GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context) override { return nullptr; }
  //!
  bool _complete = false;
  //!
  std::string _animation;
  //!
  bool _loopedAnimation;
  //!
  bool _facingRight;
  //!
  Vector2<float> _velocity;
  //!
  bool _movementType;

};

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
class LoopedAction : public AnimatedAction<Stance, Action>
{
public:
  LoopedAction(const std::string& animation, bool facingRight) :
    AnimatedAction<Stance, Action>(animation, facingRight) {}
  //!
  LoopedAction(const std::string& animation, bool facingRight, Vector2<float> instVeclocity) :
    AnimatedAction<Stance, Action>(animation, facingRight, instVeclocity) {}

  //__________________OVERRIDES________________________________

  //!
  virtual IAction* HandleInput(const InputBuffer& rawInput, const StateComponent& context) override { return nullptr; }

};

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
class StateLockedAnimatedAction : public AnimatedAction<Stance, Action>
{
public:
  //!
  StateLockedAnimatedAction(const std::string& animation, bool facingRight);
  //!
  StateLockedAnimatedAction(const std::string& animation, bool facingRight, Vector2<float> actionMovement);
  //!
  virtual IAction* HandleInput(const InputBuffer& rawInput, const StateComponent& context) override
  {
    return StateLockedHandleInput(rawInput, context, this, AnimatedAction<Stance, Action>::_complete);
  }

  virtual bool CheckInputsOnFollowUp() override { return true; }

protected:
  //!
  virtual IAction* GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context) override;

};

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
inline void AnimatedAction<Stance, Action>::Enact(Entity* actor)
{
  if (auto animator = actor->GetComponent<Animator>())
  {
    animator->ChangeListener(this);
    if (animator->AnimationLib() && animator->AnimationLib()->GetAnimation(_animation))
    {
      Animation* actionAnimation = animator->Play(_animation, _loopedAnimation, !_facingRight);

      // set the offset in the properties component (needs to be in a system)
      if (auto properties = actor->GetComponent<RenderProperties>())
      {
        properties->horizontalFlip = !_facingRight;

        if (auto rect = actor->GetComponent<Hurtbox>())
        {
          properties->offset = -animator->AnimationLib()->GetRenderOffset(_animation, !_facingRight, (int)std::floor(rect->unscaledRect.Width()));
        }
      }

      if (auto renderer = actor->GetComponent<RenderComponent<RenderType>>())
      {
        if (actionAnimation)
        {
          // render from the sheet of the new animation
          renderer->SetRenderResource(actionAnimation->GetSheetTexture<RenderType>());
          renderer->sourceRect = actionAnimation->GetFrameSrcRect(0);
        }
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
template <StanceState Stance, ActionState Action>
inline StateLockedAnimatedAction<Stance, Action>::StateLockedAnimatedAction(const std::string& animation, bool facingRight) : AnimatedAction<Stance, Action>(animation, facingRight)
{
  this->_loopedAnimation = false;
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
inline StateLockedAnimatedAction<Stance, Action>::StateLockedAnimatedAction(const std::string& animation, bool facingRight, Vector2<float> actionMovement) : AnimatedAction<Stance, Action>(animation, facingRight, actionMovement)
{
  this->_loopedAnimation = false;
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
inline IAction* StateLockedAnimatedAction<Stance, Action>::GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context)
{
  LoopedAction<Stance, ActionState::NONE> followUp(Stance == StanceState::STANDING ? "Idle" : Stance == StanceState::CROUCHING ? "Crouch" : "Jumping", this->_facingRight);
  IAction* action = followUp.HandleInput(rawInput, context);
  if (action)
    return action;
  return new LoopedAction<Stance, ActionState::NONE>(Stance == StanceState::STANDING ? "Idle" : Stance == StanceState::CROUCHING ? "Crouch" : "Jumping", this->_facingRight);
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
inline void AnimatedAction<Stance, Action>::OnAnimationComplete(const std::string& completedAnimation)
{
  if (_animation == completedAnimation)
    OnActionComplete();
}

//______________________________________________________________________________
template <> IAction* LoopedAction<StanceState::STANDING, ActionState::NONE>::HandleInput(const InputBuffer& rawInput, const StateComponent& context);

template <> IAction* LoopedAction<StanceState::JUMPING, ActionState::NONE>::HandleInput(const InputBuffer& rawInput, const StateComponent& context);

template <> IAction* LoopedAction<StanceState::CROUCHING, ActionState::NONE>::HandleInput(const InputBuffer& rawInput, const StateComponent& context);

//______________________________________________________________________________
template <> IAction* StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::NONE>::GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context);

//______________________________________________________________________________
template <> IAction* StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::NONE>::HandleInput(const InputBuffer& rawInput, const StateComponent& context);
