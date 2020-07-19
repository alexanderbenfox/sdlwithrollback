#pragma once
#include "StateMachine/AnimatedAction.h"
#include "StateMachine/ActionTimer.h"
#include "StateMachine/ActionUtil.h"

#include "Components/GameActor.h"
#include "Components/StateComponents/HitStateComponent.h"

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
  AnimatedAction<Stance, Action>::Enact(actor);
  AnimatedAction<Stance, Action>::_complete = false;
  _timer = std::shared_ptr<ActionTimer>(new SimpleActionTimer([this]() { this->OnActionComplete(); }, _duration));
  actor->GetComponent<TimerContainer>()->timings.push_back(_timer);
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
inline IAction* TimedAction<Stance, Action>::HandleInput(const InputBuffer& rawInput, const StateComponent& context)
{
  IAction* onHitAction = CheckHits(rawInput.Latest(), context, Action == ActionState::BLOCKSTUN);
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
template <StanceState Stance, ActionState Action>
class OnRecvHitAction : public TimedAction<Stance, Action>
{
public:
  //!
  OnRecvHitAction(const std::string& animation, bool facingRight, int framesInState, Vector2<float> knockback) :
    TimedAction<Stance, Action>(animation, facingRight, framesInState, knockback), _damageTaken(0) {}

  OnRecvHitAction(const std::string& animation, bool facingRight, int framesInState, Vector2<float> knockback, int damage) :
    TimedAction<Stance, Action>(animation, facingRight, framesInState, knockback), _damageTaken(damage) {}

  virtual ~OnRecvHitAction();

  //__________________OVERRIDES________________________________
  //! Adds hit state component
  virtual void Enact(Entity* actor) override;

protected:
  //! Follows up with idle state
  virtual IAction* GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context) override;
  //! Removes hit state component
  virtual void OnActionComplete() override;
  //!
  int _damageTaken = 0;
  //!
  bool _killingBlow = false;

};

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
inline OnRecvHitAction<Stance, Action>::~OnRecvHitAction()
{
  // make sure this state component is removed
  ListenedAction::_listener->GetOwner()->RemoveComponent<HitStateComponent>();
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
inline void OnRecvHitAction<Stance, Action>::Enact(Entity* actor)
{
  TimedAction<Stance, Action>::Enact(actor);

  actor->AddComponent<HitStateComponent>();
  actor->GetComponent<HitStateComponent>()->SetTimer(TimedAction<Stance, Action>::_timer.get());

  //! send damage value
  if (auto state = actor->GetComponent<StateComponent>())
  {
    if (!state->invulnerable)
    {
      state->hp -= _damageTaken;
    }
  }
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
inline IAction* OnRecvHitAction<Stance, Action>::GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context)
{
  if (context.hp <= 0)
    return new StateLockedAnimatedAction<StanceState::STANDING, ActionState::NONE>("KO", context.onLeftSide, Vector2<float>::Zero);
  return TimedAction<Stance, Action>::GetFollowUpAction(rawInput, context);
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
inline void OnRecvHitAction<Stance, Action>::OnActionComplete()
{
  ListenedAction::_listener->GetOwner()->RemoveComponent<HitStateComponent>();
  ListenedAction::OnActionComplete();
}

//______________________________________________________________________________
class ThrownAction : public TimedAction<StanceState::STANDING, ActionState::HITSTUN>
{
public:
  //!
  ThrownAction(bool facingRight, int framesInState, Vector2<float> knockback, int damage) :
    TimedAction("HeavyHitstun", facingRight, framesInState, knockback), _damageTaken(damage) {}

  virtual ~ThrownAction();

  //__________________OVERRIDES________________________________
  //! Adds hit state component
  virtual void Enact(Entity* actor) override;

protected:
  //! Follows up with idle state
  virtual IAction* GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context) override;
  //! Removes hit state component
  virtual void OnActionComplete() override;
  //!
  int _damageTaken = 0;
  //!
  bool _killingBlow = false;

};

