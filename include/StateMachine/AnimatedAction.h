#pragma once
#include "StateMachine/IAction.h"
#include "Components/Animator.h"

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
  virtual IAction* HandleInput(const InputBuffer& rawInput, const StateComponent& context) override;

  virtual bool CheckInputsOnFollowUp() override { return true; }

protected:
  //!
  virtual IAction* GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context) override;

};

//______________________________________________________________________________
template <> IAction* LoopedAction<StanceState::STANDING, ActionState::NONE>::HandleInput(const InputBuffer& rawInput, const StateComponent& context);

template <> IAction* LoopedAction<StanceState::JUMPING, ActionState::NONE>::HandleInput(const InputBuffer& rawInput, const StateComponent& context);

template <> IAction* LoopedAction<StanceState::CROUCHING, ActionState::NONE>::HandleInput(const InputBuffer& rawInput, const StateComponent& context);

//______________________________________________________________________________
template <> IAction* StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::NONE>::GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context);

//template <StanceState State, ActionState Action>
//IAction* StateLockedAnimatedAction<State, Action>::HandleInput(const InputBuffer& rawInput, const StateComponent& context);

//______________________________________________________________________________
template <> IAction* StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::NONE>::HandleInput(const InputBuffer& rawInput, const StateComponent& context);

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
inline void AnimatedAction<Stance, Action>::OnAnimationComplete(const std::string& completedAnimation)
{
  if (_animation == completedAnimation)
    OnActionComplete();
}

#ifdef _WIN32
template LoopedAction<StanceState::STANDING, ActionState::NONE>;
template StateLockedAnimatedAction<StanceState::STANDING, ActionState::NONE>;
template StateLockedAnimatedAction<StanceState::STANDING, ActionState::LIGHT>;
template StateLockedAnimatedAction<StanceState::STANDING, ActionState::MEDIUM>;
template StateLockedAnimatedAction<StanceState::STANDING, ActionState::HEAVY>;
template StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::LIGHT>;
template StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::MEDIUM>;
template StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::HEAVY>;
template StateLockedAnimatedAction<StanceState::JUMPING, ActionState::LIGHT>;
template StateLockedAnimatedAction<StanceState::JUMPING, ActionState::MEDIUM>;
template StateLockedAnimatedAction<StanceState::JUMPING, ActionState::HEAVY>;
#endif
