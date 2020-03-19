#pragma once
#include "Actions.h"

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
  void BeginNewAction(IAction* action);
  //!
  void EvaluateInputContext(InputState input, const GameContext& context);
  //!
  /*void StartAnimatedAction(const std::string& animName, bool isJC = false);
  //!
  void StartStateTransitionAction(const std::string& animName, StanceState state)
  {
    _currentActions.insert(new StateModifierAction(animName, _owner.get(), state));
  }*/

  bool IsPerformingAction() const { return _currentAction != nullptr; }

  friend std::ostream& operator<<(std::ostream& os, const GameActor& actor);
  friend std::istream& operator>>(std::istream& is, GameActor& actor);

  InputState& GetInputState() { return _lastInput; }
  GameContext& GetContext() { return _lastContext; }

  //! Context that will be merged with the input context when inputs are evaluated
  GameContext mergeContext;

  std::vector<std::shared_ptr<TimerComponent>> timings;

  IAction* const GetAction() {return _currentAction;}

private:
  //!
  IAction* _currentAction;
  //!
  std::set<IAction*> _actionsFinished;

  //!
  InputState _lastInput;
  //!
  GameContext _lastContext;
  bool _newState;

  int _comboCounter;


};

template <> struct ComponentTraits<GameActor>
{
  static const uint64_t GetSignature() { return 1 << 3;}
};
