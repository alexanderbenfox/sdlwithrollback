#pragma once
#include "StateMachine/AnimatedAction.h"

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

#ifdef _WIN32
template GroundedStaticAttack<StanceState::STANDING, ActionState::NONE>;
template AttackAction<StanceState::STANDING, ActionState::NONE>;
template AttackAction<StanceState::STANDING, ActionState::LIGHT>;
template AttackAction<StanceState::STANDING, ActionState::MEDIUM>;
template AttackAction<StanceState::STANDING, ActionState::HEAVY>;
template AttackAction<StanceState::CROUCHING, ActionState::LIGHT>;
template AttackAction<StanceState::CROUCHING, ActionState::MEDIUM>;
template AttackAction<StanceState::CROUCHING, ActionState::HEAVY>;
template AttackAction<StanceState::JUMPING, ActionState::LIGHT>;
template AttackAction<StanceState::JUMPING, ActionState::MEDIUM>;
template AttackAction<StanceState::JUMPING, ActionState::HEAVY>;
#endif
