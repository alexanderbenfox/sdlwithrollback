#pragma once
#include "StateMachine/AnimatedAction.h"
#include "StateMachine/ActionTimer.h"
#include "StateMachine/ActionUtil.h"

#include "Components/GameActor.h"
#include "Components/AttackStateComponent.h"

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
  actor->GetComponent<GameActor>()->timings.push_back(_timer);
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
inline IAction* TimedAction<Stance, Action>::HandleInput(const InputBuffer& rawInput, const StateComponent& context)
{
  if (AnimatedAction<Stance, Action>::_complete)
  {
    return GetFollowUpAction(rawInput, context);
  }

  IAction* onHitAction = CheckHits(rawInput.Latest(), context);
  if (onHitAction)
    return onHitAction;

  return nullptr;
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
inline IAction* TimedAction<Stance, Action>::GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context)
{
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
  actor->GetComponent<StateComponent>()->hp -= _damageTaken;
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
inline void OnRecvHitAction<Stance, Action>::OnActionComplete()
{
  ListenedAction::_listener->GetOwner()->RemoveComponent<HitStateComponent>();
  ListenedAction::OnActionComplete();
}