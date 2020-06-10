#pragma once
#include "Components/IComponent.h"
#include "Components/Input.h"
#include "ListenerInterfaces.h"
#include "Components/StateComponent.h"

#include <functional>

//______________________________________________________________________________
class IAction
{
public:
  virtual ~IAction() {}

  //! Begin action
  virtual void Enact(Entity* actor) = 0;
  //! return true if the input is handled only by the action
  virtual IAction* HandleInput(const InputBuffer& rawInput, const StateComponent& context) = 0;
  //!
  virtual void SetComplete() = 0;

  virtual void ChangeListener(IActionListener* listener) = 0;

  virtual StanceState GetStance() = 0;
  virtual ActionState GetAction() = 0;

  virtual bool CheckInputsOnFollowUp() = 0;

protected:
  //!
  virtual IAction* GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context) = 0;
  //!
  virtual void OnActionComplete() = 0;

};

class ActionTimer
{
public:
  ActionTimer(int duration) : 
    playTime(0),
    currFrame(0),
    _totalFrames(duration) {}

  virtual ~ActionTimer() = default;
  //! num frames the total action goes for
  int const Duration() { return _totalFrames; }


  //! gets whether or not this action has been cancelled preemptively
  virtual bool const Cancelled() = 0;
  //! callback for when the timer completes
  virtual void OnComplete() = 0;
  //! cancels the action timer
  virtual void Cancel() = 0;
  //!
  virtual void Update() = 0;

  //!
  float playTime;
  int currFrame;

protected:
  int _totalFrames;

};

//! Drives the animation - only calls the action's on complete function
class SimpleActionTimer : public ActionTimer
{
public:
  typedef std::function<void()> CompleteFunc;

  SimpleActionTimer(CompleteFunc onComplete, int duration) :
    _callback(onComplete),
    _cancelled(false),
    ActionTimer(duration) {}

  virtual bool const Cancelled() override { return _cancelled; }

  //!
  virtual void OnComplete() override
  {
    if(!_cancelled)
    {
      _callback();
    }
  }
  //!
  virtual void Cancel() override { _cancelled = true; }

  //! nothing happens on update
  virtual void Update() override {}

  
protected:
  CompleteFunc _callback;
  bool _cancelled;

};

class ComplexActionTimer : public SimpleActionTimer
{
public:
  typedef std::function<void(float, float)> TFunction;
  ComplexActionTimer(TFunction updater, CompleteFunc onComplete, int duration) :
    _updater(updater),
    SimpleActionTimer(onComplete, duration) {}

  //! updates based on the length of animation play
  virtual void Update() override 
  {
    _updater(currFrame, _totalFrames);
  }

protected:
  TFunction _updater;

};

class ListenedAction : public IAction
{
public:
  ListenedAction() : _listener(nullptr) {}
  virtual ~ListenedAction() {}

  virtual void ChangeListener(IActionListener* listener) override
  {
    _listener = listener;
  }

  virtual bool CheckInputsOnFollowUp() override { return false; }

protected:
  //!
  virtual void OnActionComplete() override
  {
    if (_listener)
      _listener->OnActionComplete(this);
  }

  //!
  IActionListener* _listener;
};

//______________________________________________________________________________
// partial specialization
template <StanceState Stance> IAction* GetAttacksFromNeutral(const InputBuffer& rawInput, bool facingRight);

//______________________________________________________________________________
IAction* CheckHits(const InputState& rawInput, const StateComponent& context);

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

  virtual void OnAnimationComplete(const std::string& completedAnimation) override
  {
    if(_animation == completedAnimation)
      OnActionComplete();
  }

  virtual void SetComplete() override { _complete = true; }

  virtual StanceState GetStance() override { return Stance; }
  virtual ActionState GetAction() override { return Action; }

protected:
  //!
  virtual IAction* GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context) override {return nullptr;}
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

//! Timed actions drive the animation/action to run for the specified duration
//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
class TimedAction : public LoopedAction<Stance, Action>
{
public:
  TimedAction(const std::string& animation, bool facingRight, int framesInState) : _duration(framesInState),
    LoopedAction<Stance, Action>(animation, facingRight) {}
  //!
  TimedAction(const std::string& animation, bool facingRight, int framesInState, Vector2<float> instVeclocity) :
    _duration(framesInState),
    LoopedAction<Stance, Action>(animation, facingRight, instVeclocity) {}

  virtual ~TimedAction();

  //__________________OVERRIDES________________________________

  //!
  virtual void Enact(Entity* actor) override;

  //! Make sure inputs are checked after follow up action since it has returned to neutral
  virtual bool CheckInputsOnFollowUp() override { return true; }

  //!
  virtual IAction* HandleInput(const InputBuffer& rawInput, const StateComponent& context) override
  {
    if(AnimatedAction<Stance, Action>::_complete)
    {
      return GetFollowUpAction(rawInput, context);
    }

    IAction* onHitAction = CheckHits(rawInput.Latest(), context);
    if (onHitAction)
      return onHitAction;
    
    return nullptr;
  }

protected:
  //! Follows up with idle state
  virtual IAction* GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context) override;

  std::shared_ptr<ActionTimer> _actionTiming;
  int _duration;

};

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
class DashAction : public TimedAction<Stance, Action>
{
public:
  //!
  DashAction(const std::string& animation, bool facingRight, int framesInState, float dashSpeed) :
    _dashSpeed(dashSpeed),
    TimedAction<Stance, Action>(animation, facingRight, framesInState) {}

  void Enact(Entity* actor) override;

private:

  float _dashSpeed;

};

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
class OnRecvHitAction : public TimedAction<Stance, Action>
{
public:
  //!
  OnRecvHitAction(const std::string& animation, bool facingRight, int framesInState, Vector2<float> instVeclocity) :
    TimedAction<Stance, Action>(animation, facingRight, framesInState, instVeclocity), _damageTaken(0) {}

  OnRecvHitAction(const std::string& animation, bool facingRight, int framesInState, Vector2<float> instVeclocity, int damage) :
    TimedAction<Stance, Action>(animation, facingRight, framesInState, instVeclocity), _damageTaken(damage) {}

  virtual ~OnRecvHitAction();

  //__________________OVERRIDES________________________________
  //! Adds hit state component
  virtual void Enact(Entity* actor) override;

protected:
  
  //! Removes hit state component
  virtual void OnActionComplete() override;
  //!
  int _damageTaken = 0;

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
template <StanceState Stance, ActionState Action>
class AttackAction : public StateLockedAnimatedAction<Stance, Action>
{
public:
  //!
  AttackAction(const std::string& animation, bool facingRight) :
    StateLockedAnimatedAction<Stance, Action>(animation, facingRight) {}
  //!
  AttackAction(const std::string& animation, bool facingRight, Vector2<float> actionMovement) :
    StateLockedAnimatedAction<Stance, Action>(animation, facingRight, actionMovement) {}

  virtual ~AttackAction();

  //! Adds attack state component
  virtual void Enact(Entity* actor) override;

protected:

  //! Removes attack state component
  virtual void OnActionComplete() override;

  virtual IAction* GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context) override
  {
    return new LoopedAction<Stance, ActionState::NONE>(Stance == StanceState::STANDING ? "Idle" : Stance == StanceState::CROUCHING ? "Crouch" : "Jumping", this->_facingRight);
  }

};

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
class GroundedStaticAttack : public AttackAction<Stance, Action>
{
public:
  //!
  GroundedStaticAttack(const std::string& animation, bool facingRight) : AttackAction<Stance, Action>(animation, facingRight, Vector2<float>(0, 0)) {}

};

//______________________________________________________________________________
template <> IAction* LoopedAction<StanceState::STANDING, ActionState::NONE>::HandleInput(const InputBuffer& rawInput, const StateComponent& context);

template <> IAction* LoopedAction<StanceState::JUMPING, ActionState::NONE>::HandleInput(const InputBuffer& rawInput, const StateComponent& context);

template <> IAction* LoopedAction<StanceState::CROUCHING, ActionState::NONE>::HandleInput(const InputBuffer& rawInput, const StateComponent& context);

//______________________________________________________________________________
template <> IAction* StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::NONE>::GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context);

template <StanceState State, ActionState Action>
inline IAction* StateLockedAnimatedAction<State, Action>::HandleInput(const InputBuffer& rawInput, const StateComponent& context)
{
  //!!!! TESTING SPECIAL MOVE CANCELS HERE
  if (context.hitting)
  {
    if (HasState(rawInput.Latest(), InputState::BTN1) || HasState(rawInput.Latest(), InputState::BTN2) || HasState(rawInput.Latest(), InputState::BTN3))
    {
      bool qcf = rawInput.Evaluate(UnivSpecMoveDict) == SpecialMoveState::QCF && context.onLeftSide;
      bool qcb = rawInput.Evaluate(UnivSpecMoveDict) == SpecialMoveState::QCB && !context.onLeftSide;
      if (qcf || qcb)
        return new GroundedStaticAttack<StanceState::STANDING, ActionState::NONE>("SpecialMove1", context.onLeftSide);
    }
  }

  if(AnimatedAction<State, Action>::_complete)
  {
    return GetFollowUpAction(rawInput, context);
  }

  if (context.hitThisFrame)
  {
    int neutralFrame = context.frameData.active + context.frameData.recover + 1;
    return new OnRecvHitAction<StanceState::STANDING, ActionState::HITSTUN>("HeavyHitstun", context.onLeftSide, neutralFrame + context.frameData.onHitAdvantage, context.frameData.knockback);
  }

  if (State == StanceState::JUMPING)
  {
    if (HasState(context.collision, CollisionSide::DOWN))
    {
      return GetFollowUpAction(rawInput, context);
    }
  }
  return nullptr;
}

//______________________________________________________________________________
template <> IAction* StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::NONE>::HandleInput(const InputBuffer& rawInput, const StateComponent& context);

#ifdef _WIN32
template LoopedAction<StanceState::STANDING, ActionState::NONE>;
#endif