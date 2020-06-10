#pragma once
#include "Actions.h"
#include "StateComponent.h"

#include <set>
#include "ListenerInterfaces.h"

class GameActor : public IComponent, public IActionListener
{
public:
  //!
  GameActor(std::shared_ptr<Entity> owner);
  //!
  virtual void OnFrameBegin() override {}
  //! Finishes all of the completed actions in the queue
  virtual void OnFrameEnd() override;
  //! 
  virtual void OnActionComplete(IAction* action) override;
  //!
  virtual Entity* GetOwner() override { return _owner.get(); }
  //!
  virtual void SetStateInfo(StanceState stance, ActionState action) override
  {
    _currAction = action;
    _currStance = stance;
  }
  //!

  //!
  void BeginNewAction(IAction* action);
  //!
  void EvaluateInputContext(const InputBuffer& input, const StateComponent* stateInfo, float dt);
  //!
  bool IsPerformingAction() const { return _currentAction != nullptr; }

  friend std::ostream& operator<<(std::ostream& os, const GameActor& actor);
  friend std::istream& operator>>(std::istream& is, GameActor& actor);

  InputState& GetInputState() { return _lastInput; }
  //GameContext& GetContext() { return _lastContext; }

  //! Context that will be merged with the input context when inputs are evaluated
  //GameContext mergeContext;

  std::vector<std::shared_ptr<ActionTimer>> timings;

  //IAction* const GetAction() {return _currentAction;}
  StanceState const& GetStanceState() { return _currStance; }
  ActionState const& GetActionState() { return _currAction; }

private:
  //!
  IAction* _currentAction;
  //!
  std::set<IAction*> _actionsFinished;

  //!
  InputState _lastInput;
  //!
  StateComponent _lastState;

  bool _newState;

  int _comboCounter;
  std::shared_ptr<Entity> _counterText;

  StanceState _currStance;
  ActionState _currAction;


};
