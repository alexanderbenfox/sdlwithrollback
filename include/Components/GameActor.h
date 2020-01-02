#pragma once
#include "IComponent.h"
#include "Actions.h"

#include <set>

class GameActor : public IComponent, public IActionListener
{
public:
  //!
  GameActor(std::shared_ptr<Entity> owner);
  //!
  virtual void Update(float dt) override;
  //!
  virtual void OnFrameBegin() override {}
  //! Finishes all of the completed actions in the queue
  virtual void OnFrameEnd() override;
  //! 
  virtual void OnActionComplete(IAction* action) override;
  //!
  void HandleInput(InputState input);
  //!
  /*void StartAnimatedAction(const std::string& animName, bool isJC = false);
  //!
  void StartStateTransitionAction(const std::string& animName, StanceState state)
  {
    _currentActions.insert(new StateModifierAction(animName, _owner.get(), state));
  }*/

  void BeginNewAction(IAction* action)
  {
    if (_currentAction != nullptr)
      delete _currentAction;
    _currentAction = action;
  }

  bool IsPerformingAction() const { return _currentAction != nullptr; }

private:
  //!
  IAction* _currentAction;
  //!
  std::set<IAction*> _actionsFinished;

  //!
  InputState _lastInput;
  CollisionSide _lastCollision;
  bool _newState;


};