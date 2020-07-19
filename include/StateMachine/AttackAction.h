#pragma once
#include "StateMachine/AnimatedAction.h"
#include "Components/StateComponents/AttackStateComponent.h"

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
  //! Checks for special cancels
  virtual IAction* HandleInput(const InputBuffer& rawInput, const StateComponent& context) override;
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
  virtual ~GroundedStaticAttack() = default;
};

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
class SpecialMoveAttack : public AttackAction<Stance, Action>
{
public:
  //!
  SpecialMoveAttack(const std::string& animation, bool facingRight) : AttackAction<Stance, Action>(animation, facingRight, Vector2<float>(0, 0)) {}
  //! Does not check for special cancels
  virtual IAction* HandleInput(const InputBuffer& rawInput, const StateComponent& context) override
  {
    return StateLockedAnimatedAction<Stance, Action>::HandleInput(rawInput, context);
  }
};

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
inline AttackAction<Stance, Action>::~AttackAction()
{
  // make sure this state component is removed
  ListenedAction::_listener->GetOwner()->RemoveComponent<AttackStateComponent>();
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
inline void AttackAction<Stance, Action>::Enact(Entity* actor)
{
  AnimatedAction<Stance, Action>::Enact(actor);
  if (auto animator = actor->GetComponent<Animator>())
  {
    if (animator->AnimationLib()->GetAnimation(AnimatedAction<Stance, Action>::_animation) && animator->AnimationLib()->GetEventList(AnimatedAction<Stance, Action>::_animation))
    {
      actor->AddComponent<AttackStateComponent>();
      actor->GetComponent<AttackStateComponent>()->Init(animator->AnimationLib()->GetAnimation(AnimatedAction<Stance, Action>::_animation), animator->AnimationLib()->GetEventList(AnimatedAction<Stance, Action>::_animation));
    }
  }
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
inline IAction* AttackAction<Stance, Action>::HandleInput(const InputBuffer& rawInput, const StateComponent& context)
{
  IAction* action = nullptr;

  // if we are hitting, we can cancel the remaining recovery and active frames into a special move
  if (context.hitting)
    action = CheckSpecials(rawInput, context);

  if (!action)
    return StateLockedAnimatedAction<Stance, Action>::HandleInput(rawInput, context);
  return action;
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
inline void AttackAction<Stance, Action>::OnActionComplete()
{
  ListenedAction::_listener->GetOwner()->RemoveComponent<AttackStateComponent>();
  StateLockedAnimatedAction<Stance, Action>::OnActionComplete();
}

//______________________________________________________________________________
class ThrowInitateAction : public GroundedStaticAttack<StanceState::STANDING, ActionState::NONE>
{
public:
  //!
  ThrowInitateAction(bool fThrow, bool facingRight) :
    GroundedStaticAttack<StanceState::STANDING, ActionState::NONE>(fThrow ? "ForwardThrow" : "BackThrow", facingRight) {}


  virtual ~ThrowInitateAction()
  {
    ListenedAction::_listener->GetOwner()->GetComponent<Rigidbody>()->ignoreDynamicColliders = true;
  }

  //! Sets dynamic collision detection to false
  virtual void Enact(Entity* actor) override
  {
    actor->GetComponent<Rigidbody>()->ignoreDynamicColliders = true;
    GroundedStaticAttack<StanceState::STANDING, ActionState::NONE>::Enact(actor);
  }

  //! Checks for if the throw was successful
  virtual IAction* HandleInput(const InputBuffer& rawInput, const StateComponent& context) override
  {
    // if throw came out this frame and it wasn't a success, go to throw miss
    if (context.triedToThrowThisFrame && !context.throwSuccess)
    {
      return new StateLockedAnimatedAction("ThrowMiss", context.onLeftSide);
    }
    return StateLockedAnimatedAction::HandleInput(rawInput, context);
  }
};
