#pragma once
#include "StateMachine/AnimatedAction.h"
#include "StateMachine/ActionTimer.h"
#include "Components/GameActor.h"

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

#ifdef _WIN32
template TimedAction<StanceState::STANDING, ActionState::BLOCKSTUN>;
template TimedAction<StanceState::STANDING, ActionState::HITSTUN>;
template TimedAction<StanceState::STANDING, ActionState::DASHING>;
template OnRecvHitAction<StanceState::STANDING, ActionState::BLOCKSTUN>;
template OnRecvHitAction<StanceState::STANDING, ActionState::HITSTUN>;
template DashAction<StanceState::STANDING, ActionState::DASHING>;
#endif
