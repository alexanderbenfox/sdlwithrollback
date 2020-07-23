#include "StateMachine/TimedAction.h"

#include "Components/Rigidbody.h"
#include "Components/GameActor.h"
#include "Components/Animator.h"

//______________________________________________________________________________
void DashAction::Enact(Entity* actor)
{
  this->_loopedAnimation = false;
  AnimatedAction<StanceState::STANDING, ActionState::DASHING>::Enact(actor);
  AnimatedAction<StanceState::STANDING, ActionState::DASHING>::_complete = false;

  // adjust speed of animation to match the duration of action
  auto animator = actor->GetComponent<Animator>();
  animator->playSpeed = static_cast<float>(animator->GetCurrentAnimation().GetFrameCount()) / static_cast<float>(TimedAction<StanceState::STANDING, ActionState::DASHING>::_duration);

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
  TimedAction<StanceState::STANDING, ActionState::DASHING>::_timer = std::shared_ptr<ActionTimer>(new ComplexActionTimer(
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
    TimedAction<StanceState::STANDING, ActionState::DASHING>::_duration));

  actor->GetComponent<TimerContainer>()->timings.push_back(TimedAction<StanceState::STANDING, ActionState::DASHING>::_timer);
}

//______________________________________________________________________________
IAction* DashAction::GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context)
{
  return new LoopedAction<StanceState::STANDING, ActionState::NONE>("Idle", this->_facingRight);
}

//______________________________________________________________________________
ThrownAction::~ThrownAction()
{
  // make sure this state component is removed
  ListenedAction::_listener->GetOwner()->RemoveComponent<ThrownStateComponent>();
  ListenedAction::_listener->GetOwner()->GetComponent<Rigidbody>()->ignoreDynamicColliders = false;
  _delayTimer->Cancel();
}

//______________________________________________________________________________
inline void ThrownAction::Enact(Entity* actor)
{
  TimedAction::Enact(actor);

  actor->AddComponent<ThrownStateComponent>();
  actor->GetComponent<ThrownStateComponent>()->SetTimer(TimedAction::_timer.get());

  actor->GetComponent<StateComponent>()->thrownThisFrame = false;
  actor->GetComponent<Rigidbody>()->enabled = false;

  auto rb = actor->GetComponent<Rigidbody>();

  _delayTimer = std::shared_ptr<ActionTimer>(
    new SimpleActionTimer([rb, this]()
      {
        rb->_vel = _velocity;
        rb->enabled = true;
      }, _delay));
  actor->GetComponent<TimerContainer>()->timings.push_back(_delayTimer);

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
IAction* ThrownAction::GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context)
{
  if (context.hp <= 0)
    return new StateLockedAnimatedAction<StanceState::STANDING, ActionState::NONE>("KO", context.onLeftSide, Vector2<float>::Zero);
  return TimedAction::GetFollowUpAction(rawInput, context);
}

//______________________________________________________________________________
void ThrownAction::OnActionComplete()
{
  ListenedAction::_listener->GetOwner()->RemoveComponent<ThrownStateComponent>();
  ListenedAction::OnActionComplete();
}
