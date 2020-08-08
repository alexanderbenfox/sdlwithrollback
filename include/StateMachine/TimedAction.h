#pragma once
#include "StateMachine/AnimatedAction.h"
#include "StateMachine/ActionUtil.h"

#include "Components/TimerContainer.h"
#include "Components/StateComponents/HitStateComponent.h"
#include "Components/SFXComponent.h"

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
  virtual IAction* HandleInput(const InputBuffer& rawInput, const StateComponent& context) override;

  virtual void Serialize(std::ostream& os) override
  {
    AnimatedAction<Stance, Action>::Serialize(os);
    os << _duration;
  }

  virtual void Deserialize(std::istream& is) override
  {
    AnimatedAction<Stance, Action>::Deserialize(is);
    is >> _duration;
  }

protected:
  //! Follows up with idle state
  virtual IAction* GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context) override;
  //!
  std::shared_ptr<ActionTimer> _timer;
  //!
  int _duration;

};

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
inline TimedAction<Stance, Action>::~TimedAction<Stance, Action>()
{
  _timer->Cancel();
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
inline void TimedAction<Stance, Action>::Enact(Entity* actor)
{
  AnimatedAction<Stance, Action>::_forceAnimRestart = true;
  AnimatedAction<Stance, Action>::Enact(actor);
  AnimatedAction<Stance, Action>::_complete = false;
  _timer = std::shared_ptr<ActionTimer>(new SimpleActionTimer([this]() { this->OnActionComplete(); }, _duration));
  actor->GetComponent<TimerContainer>()->timings.push_back(_timer);
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
inline IAction* TimedAction<Stance, Action>::HandleInput(const InputBuffer& rawInput, const StateComponent& context)
{
  IAction* onHitAction = CheckHits(rawInput.Latest(), context, Action == ActionState::BLOCKSTUN, false);
  if (onHitAction) return onHitAction;

  if (AnimatedAction<Stance, Action>::_complete)
  {
    return GetFollowUpAction(rawInput, context);
  }

  return nullptr;
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
inline IAction* TimedAction<Stance, Action>::GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context)
{
  LoopedAction<Stance, ActionState::NONE> followUp(Stance == StanceState::STANDING ? "Idle" : Stance == StanceState::CROUCHING ? "Crouch" : "Jumping", this->_facingRight);
  IAction* action = followUp.HandleInput(rawInput, context);
  if (action)
    return action;
  return new LoopedAction<Stance, ActionState::NONE>(Stance == StanceState::STANDING ? "Idle" : Stance == StanceState::CROUCHING ? "Crouch" : "Jumping", this->_facingRight);
}

//______________________________________________________________________________
class DashAction : public TimedAction<StanceState::STANDING, ActionState::DASHING>
{
public:
  //!
  DashAction(const std::string& animation, bool facingRight, int framesInState, float dashSpeed) :
    _dashSpeed(dashSpeed),
    TimedAction<StanceState::STANDING, ActionState::DASHING>(animation, facingRight, framesInState) {}

  void Enact(Entity* actor) override;

protected:
  //! Follows up with idle state
  virtual IAction* GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context) override;

  float _dashSpeed;

};

//______________________________________________________________________________
class GrappledAction : public TimedAction<StanceState::STANDING, ActionState::HITSTUN>
{
public:
  //!
  GrappledAction(bool facingRight, int framesInState, Vector2<float> knockback, int damage, int framesTilThrow) :
    TimedAction("HeavyHitstun", facingRight, framesInState, knockback), _damageTaken(damage), _delay(framesTilThrow) {}

  virtual ~GrappledAction();

  //__________________OVERRIDES________________________________
  //! Adds hit state component
  virtual void Enact(Entity* actor) override;

  virtual void Serialize(std::ostream& os) override
  {
    TimedAction<StanceState::STANDING, ActionState::HITSTUN>::Serialize(os);
    os << _damageTaken;
    os << _killingBlow;
    os << _delay;
  }

  virtual void Deserialize(std::istream& is) override
  {
    TimedAction<StanceState::STANDING, ActionState::HITSTUN>::Deserialize(is);
    is >> _damageTaken;
    is >> _killingBlow;
    is >> _delay;
  }

protected:
  //! Follows up with idle state
  virtual IAction* GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context) override;
  //! Removes hit state component
  virtual void OnActionComplete() override;
  //!
  int _damageTaken = 0;
  //!
  bool _killingBlow = false;
  //! Timer for delaying knockback
  std::shared_ptr<ActionTimer> _delayTimer;
  //! Frames until knockback
  int _delay;

};

