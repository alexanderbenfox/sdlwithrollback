#pragma once
#include "Components/IComponent.h"
#include "Components/Input.h"
#include "ListenerInterfaces.h"


//! Drives the animation
class TimerComponent
{
public:
  TimerComponent(std::function<void()> onComplete, int duration) : playTime(0), currFrame(0), _totalFrames(duration), _onComplete(onComplete), _cancelled(false) {}
  float playTime;
  int currFrame;

  //!
  int const TotalFrames() { return _totalFrames; }
  //!
  void OnComplete()
  {
    if(!_cancelled)
      _onComplete();
  }
  //!
  void Cancel() { _cancelled = true; }

  bool const Cancelled() { return _cancelled; }
private:
  int _totalFrames;
  std::function<void()> _onComplete;
  bool _cancelled;

};

//______________________________________________________________________________
//! Inteface for a context of the world that may affect character state
class GameContext
{
public:
  GameContext() : collision(CollisionSide::NONE), onLeftSide(false) {}
  ~GameContext() {}

  bool operator==(const GameContext& other) const
  {
    return collision == other.collision && onLeftSide == other.onLeftSide && movement == other.movement && hitThisFrame == other.hitThisFrame;
  }

  // will merge the contexts?
  GameContext operator+(const GameContext& otherContext) const
  {
    // copy this
    GameContext newContext = *this;
    newContext.hitThisFrame |= otherContext.hitThisFrame;
    newContext.hitOnLeftSide |= otherContext.hitOnLeftSide;
    newContext.frameData = otherContext.frameData;
    return newContext;
  }

  Vector2<float> movement;
  CollisionSide collision;
  bool onLeftSide;
  bool hitThisFrame = false;
  bool hitOnLeftSide = false;
  FrameData frameData;

};

//______________________________________________________________________________
class IAction
{
public:
  virtual ~IAction() {}

  //! Begin action
  virtual void Enact(Entity* actor) = 0;
  //! return true if the input is handled only by the action
  virtual IAction* HandleInput(const InputState& rawInput, const GameContext& context) = 0;
  //!
  virtual void SetComplete() = 0;

  virtual void ChangeListener(IActionListener* listener) = 0;

  virtual StanceState GetStance() = 0;
  virtual ActionState GetAction() = 0;

protected:
  //!
  virtual IAction* GetFollowUpAction() = 0;
  //!
  virtual void OnActionComplete() = 0;

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
template <StanceState Stance> IAction* GetAttacksFromNeutral(const InputState& rawInput, bool facingRight);

//______________________________________________________________________________
IAction* CheckHits(const InputState& rawInput, const GameContext& context);

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
  virtual IAction* HandleInput(const InputState& rawInput, const GameContext& context) override = 0;

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
  virtual IAction* GetFollowUpAction() override {return nullptr;}
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
  virtual IAction* HandleInput(const InputState& rawInput, const GameContext& context) override { return nullptr; }

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

  //!
  virtual IAction* HandleInput(const InputState& rawInput, const GameContext& context) override
  {
    if(AnimatedAction<Stance, Action>::_complete)
    {
      return GetFollowUpAction();
    }
    
    IAction* onHitAction = CheckHits(rawInput, context);
    if (onHitAction)
      return onHitAction;
    return nullptr;
  }

protected:
  virtual IAction* GetFollowUpAction() override = 0;

  std::shared_ptr<TimerComponent> _actionTiming;
  int _duration;

};

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
class OnRecvHitAction : public TimedAction<Stance, Action>
{
public:
  //!
  OnRecvHitAction(const std::string& animation, bool facingRight, int framesInState, Vector2<float> instVeclocity) :
    TimedAction<Stance, Action>(animation, facingRight, framesInState, instVeclocity) {}

  virtual ~OnRecvHitAction();

  //__________________OVERRIDES________________________________
  //! Adds hit state component
  virtual void Enact(Entity* actor) override;

protected:
  //! Follows up with idle state
  virtual IAction* GetFollowUpAction() override;
  //! Removes hit state component
  virtual void OnActionComplete() override;

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
  virtual IAction* HandleInput(const InputState& rawInput, const GameContext& context) override;

protected:
  //!
  virtual IAction* GetFollowUpAction() override;
  
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
template <> IAction* LoopedAction<StanceState::STANDING, ActionState::NONE>::HandleInput(const InputState& rawInput, const GameContext& context);

template <> IAction* LoopedAction<StanceState::JUMPING, ActionState::NONE>::HandleInput(const InputState& rawInput, const GameContext& context);

template <> IAction* LoopedAction<StanceState::CROUCHING, ActionState::NONE>::HandleInput(const InputState& rawInput, const GameContext& context);


//______________________________________________________________________________
template <> IAction* StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::NONE>::GetFollowUpAction();

template <StanceState State, ActionState Action>
inline IAction* StateLockedAnimatedAction<State, Action>::HandleInput(const InputState& rawInput, const GameContext& context)
{
  if(AnimatedAction<State, Action>::_complete)
  {
    return GetFollowUpAction();
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
      return GetFollowUpAction();
    }
  }
  return nullptr;
}

#ifdef _WIN32
template LoopedAction<StanceState::STANDING, ActionState::NONE>;
#endif