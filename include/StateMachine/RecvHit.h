#pragma once
#include "TimedAction.h"

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
  //! Creates blockstun sfx and transfers damage
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

  //! send damage value
  if (auto state = actor->GetComponent<StateComponent>())
  {
    if (!state->invulnerable)
    {
      state->hp -= _damageTaken;
    }
  }

  if (Action == ActionState::BLOCKSTUN)
  {
    actor->GetComponent<SFXComponent>()->ShowBlockSparks();
  }
  else if (Action == ActionState::HITSTUN)
  {
    actor->GetComponent<SFXComponent>()->ShowHitSparks();
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
template <StanceState Stance, ActionState Action>
class HitOrBlockStunAction : public OnRecvHitAction<Stance, Action>
{
public:
  HitOrBlockStunAction(const std::string& animation, bool facingRight, int framesInState, Vector2<float> knockback, int damage) :
    OnRecvHitAction<Stance, Action>(animation, facingRight, framesInState, knockback, damage) {}

  virtual void Enact(Entity* actor) override
  {
    OnRecvHitAction<Stance, Action>::Enact(actor);
    actor->AddComponent<HitStateComponent>();
    actor->GetComponent<HitStateComponent>()->SetTimer(TimedAction<Stance, Action>::_timer.get());
  }
  
};

// action state for being airborne in knockdown but not grounded yet
//______________________________________________________________________________
class KnockdownAirborneAction : public OnRecvHitAction<StanceState::KNOCKDOWN, ActionState::HITSTUN>
{
public:
  //!
  KnockdownAirborneAction(bool facingRight, Vector2<float> knockback, int damage) : OnRecvHitAction("Knockdown_Air", facingRight, 0, knockback, damage) {}

  //__________________OVERRIDES________________________________
  virtual IAction* HandleInput(const InputBuffer& rawInput, const StateComponent& context) override
  {
    // do nothing on animation complete
    /*if (AnimatedAction::_complete)
    {

    }*/

    // check to see if we've been hit again
    if (IAction* action = CheckHits(rawInput.Latest() , context, false, true)) return action;
    // if we've hit the ground, transition to grounded knockdown state
    if (HasState(context.collision, CollisionSide::DOWN))
    {
      return new StateLockedAnimatedAction<StanceState::KNOCKDOWN, ActionState::HITSTUN>("Knockdown_HitGround", context.onLeftSide, Vector2<float>::Zero);
    }
    return nullptr;
  }

protected:
  //! No follow up action unless it has hit the ground
  virtual IAction* GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context) override { return nullptr; }

};