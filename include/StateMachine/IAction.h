#pragma once
#include "Components/StateComponent.h"
#include "Components/Input.h"

//______________________________________________________________________________
//all of the possible states for animation??
enum class StanceState
{
  CROUCHING, STANDING, JUMPING
};

//______________________________________________________________________________
enum class ActionState
{
  NONE, BLOCKSTUN, HITSTUN, DASHING, LIGHT, MEDIUM, HEAVY
};


class IAction;

//______________________________________________________________________________
class IActionListener
{
public:
  virtual void OnActionComplete(IAction*) = 0;

  virtual Entity* GetOwner() = 0;

  virtual void SetStateInfo(StanceState stance, ActionState action) = 0;
};

//______________________________________________________________________________
class IAnimatorListener
{
public:
  virtual void OnAnimationComplete(const std::string& completedAnimation) = 0;
};

//______________________________________________________________________________
class IAction
{
public:
  virtual ~IAction() {}

  //! Begin action
  virtual void Enact(Entity* actor) = 0;
  //! return true if the input is handled only by the action
  virtual IAction* HandleInput(const InputBuffer& rawInput, const StateComponent& context) = 0;
  //!
  virtual void SetComplete() = 0;

  virtual void ChangeListener(IActionListener* listener) = 0;

  virtual StanceState GetStance() = 0;
  virtual ActionState GetAction() = 0;

  virtual bool CheckInputsOnFollowUp() = 0;

  //!
  virtual IAction* GetFollowUpAction(const InputBuffer& rawInput, const StateComponent& context) = 0;

protected:
  //!
  virtual void OnActionComplete() = 0;

};

//______________________________________________________________________________
class ListenedAction : public IAction
{
public:
  ListenedAction() : _listener(nullptr) {}
  virtual ~ListenedAction() {}

  virtual void ChangeListener(IActionListener* listener) override { _listener = listener; }

  virtual bool CheckInputsOnFollowUp() override { return false; }

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

