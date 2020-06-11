#include "StateMachine/TimedAction.h"
#include "StateMachine/ActionUtil.h"

#include "Components/AttackStateComponent.h"
#include "Components/Rigidbody.h"
#include "Components/GameActor.h"

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
TimedAction<Stance, Action>::~TimedAction<Stance, Action>()
{
  _timer->Cancel();
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
void TimedAction<Stance, Action>::Enact(Entity* actor)
{
  AnimatedAction<Stance, Action>::Enact(actor);
  AnimatedAction<Stance, Action>::_complete = false;
  _timer = std::shared_ptr<ActionTimer>(new SimpleActionTimer([this]() { this->OnActionComplete(); }, _duration));
  actor->GetComponent<GameActor>()->timings.push_back(_timer);
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
IAction* TimedAction<Stance, Action>::HandleInput(const InputBuffer& rawInput, const StateComponent& context)
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
IAction* TimedAction<Stance, Action>::GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context)
{
  return new LoopedAction<Stance, ActionState::NONE>(Stance == StanceState::STANDING ? "Idle" : Stance == StanceState::CROUCHING ? "Crouch" : "Jumping", this->_facingRight);
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
void DashAction<Stance, Action>::Enact(Entity* actor)
{
  this->_loopedAnimation = false;
  AnimatedAction<Stance, Action>::Enact(actor);
  AnimatedAction<Stance, Action>::_complete = false;

  // adjust speed of animation to match the duration of action
  auto animator = actor->GetComponent<Animator>();
  animator->playSpeed = static_cast<float>(animator->GetCurrentAnimation().GetFrameCount()) / static_cast<float>(TimedAction<Stance, Action>::_duration);

  // need to get rigid body to move character around during the timer
  std::shared_ptr<Rigidbody> rb = actor->GetComponent<Rigidbody>();

  std::function<void(float, float)> parabola = [rb, this](float t, float totalT)
  {
    // make peak speed at halfway through action
    float t2 = (t - totalT / 2.0f) * (t - totalT / 2.0f);
    // send velocity to rigidbody component based on inv parabola
    rb->_vel = Vector2<float>(-t2 + _dashSpeed, 0);
  };

  // initialize the update functions
  TimedAction<Stance, Action>::_timer = std::shared_ptr<ActionTimer>(new ComplexActionTimer(
    [rb, this](float t, float totalT)
    {
      float f_t = Interpolation::Plateau::F(t, totalT, _dashSpeed);
      rb->_vel = Vector2<float>(f_t, 0);
    },
    [rb, this]()
    {
      this->OnActionComplete();
      // make sure movement is stopped
      rb->_vel = Vector2<float>(0, 0);
    },
    TimedAction<Stance, Action>::_duration));

  actor->GetComponent<GameActor>()->timings.push_back(TimedAction<Stance, Action>::_timer);
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
  actor->GetComponent<HitStateComponent>()->SetTimer(TimedAction<Stance, Action>::_timer.get());

  //! send damage value
  actor->GetComponent<StateComponent>()->hp -= _damageTaken;
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
void OnRecvHitAction<Stance, Action>::OnActionComplete()
{
  ListenedAction::_listener->GetOwner()->RemoveComponent<HitStateComponent>();
  ListenedAction::OnActionComplete();
}
