#pragma once
#include "StateMachine/IAction.h"
#include "StateMachine/ActionTimer.h"

#include "Components/StateComponent.h"

#include <set>

class GameActor : public IComponent, public IActionListener
{
public:
  //!
  GameActor(std::shared_ptr<Entity> owner);
  //!
  ~GameActor();
  //! 
  virtual void OnActionComplete(IAction* action) override;
  //!
  virtual std::shared_ptr<Entity> GetOwner() override { return _owner; }
  //!
  virtual void SetStateInfo(StanceState stance, ActionState action) override;
  //!
  void BeginNewAction(IAction* action);
  //! returns true if entered a new action
  bool EvaluateInputContext(const InputBuffer& input, const StateComponent* stateInfo);

  StanceState const& GetStanceState() { return _currStance; }
  ActionState const& GetActionState() { return _currAction; }

  friend std::ostream& operator<<(std::ostream& os, const GameActor& actor);
  friend std::istream& operator>>(std::istream& is, GameActor& actor);

  bool actionTimerComplete = false;

  void TransferInputData(const InputBuffer& buffer, const StateComponent* stateInfo)
  {
    newInputs = false;
    if (forceNewInputOnNextFrame || buffer.Latest() != _lastInput || buffer.GetLastSpecialInput() != _lastSpInput)
    {
      _lastInput = buffer.Latest();
      _lastSpInput = buffer.GetLastSpecialInput();
      newInputs = true;
    }
    forceNewInputOnNextFrame = false;

    if (stateInfo->collision != _lastState.collision || stateInfo->onNewState)
    {
      _lastState = *stateInfo;
      newInputs = true;
    }
  }

  bool newInputs = true;
  bool forceNewInputOnNextFrame = false;

  InputState const& LastButtons() { return _lastInput; }
  SpecialInputState const& LastSpecial() { return _lastSpInput; }

private:
  //!
  IAction* _currentAction;

  //!
  InputState _lastInput;
  SpecialInputState _lastSpInput;

  //!
  StateComponent _lastState;

  bool _newState;
  

  StanceState _currStance;
  ActionState _currAction;

};
