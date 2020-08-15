#pragma once
#include "Components/StateComponent.h"
#include "Components/InputHandlers/InputBuffer.h"

class GameActor : public IComponent
{
public:
  GameActor();
  //!
  StanceState const& GetStanceState() { return _currStance; }
  ActionState const& GetActionState() { return _currAction; }

  friend std::ostream& operator<<(std::ostream& os, const GameActor& actor);
  friend std::istream& operator>>(std::istream& is, GameActor& actor);

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
  //! indicates current action is complete and entity should trigger its "TransitionTo" action if it has one
  bool actionTimerComplete = false;

  InputState const& LastButtons() { return _lastInput; }
  SpecialInputState const& LastSpecial() { return _lastSpInput; }

private:
  //!
  InputState _lastInput;
  SpecialInputState _lastSpInput;

  //!
  StateComponent _lastState;

  bool _newState;
  

  StanceState _currStance;
  ActionState _currAction;

};

//! Empty flag for identifying entity as an actor in the scene
struct Actor : public IComponent {};
