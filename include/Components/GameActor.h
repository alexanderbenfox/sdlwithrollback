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
  //! returns true if entered a new action
  bool EvaluateInputContext(const InputBuffer& input, const StateComponent* stateInfo);
  //!
  bool IsPerformingAction() const { return _currentAction != nullptr; }

  friend std::ostream& operator<<(std::ostream& os, const GameActor& actor);
  friend std::istream& operator>>(std::istream& is, GameActor& actor);

  InputState& GetInputState() { return _lastInput; }
  //GameContext& GetContext() { return _lastContext; }

  //! Context that will be merged with the input context when inputs are evaluated
  //GameContext mergeContext;

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

  StanceState _currStance;
  ActionState _currAction;


};

struct TimerContainer : public IComponent
{
  TimerContainer(std::shared_ptr<Entity> owner) : IComponent(owner) {}
  virtual ~TimerContainer()
  {
    for(auto timing : timings)
      timing->OnComplete();
  }
  std::vector<std::shared_ptr<ActionTimer>> timings;
};
