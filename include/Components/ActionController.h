#pragma once
#include "IComponent.h"
#include "Actions.h"

#include <set>

class ActionController : public IComponent, public IActionListener
{
public:
  ActionController(std::shared_ptr<Entity> owner) : IComponent(owner) {}
  //virtual ~IComponent() = 0;
  //template<typename... Args> virtual void Init(Args... params) {}
  //!
  virtual void Update(float dt) override;
  //!
  virtual void OnFrameBegin() override {}
  //!
  virtual void OnFrameEnd() override;

  virtual void OnActionComplete(IAction* action) override;

  bool HandleInput(InputState bttn);

  void StartAnimatedAction(const std::string& animName, bool isJC = false);

  void StartStateTransitionAction(const std::string& animName, StanceState state)
  {
    _currentActions.insert(new StateModifierAction(animName, _owner.get(), state));
  }

  bool IsPerformingAction() const { return !_currentActions.empty(); }

private:
  //!
  std::set<IAction*> _currentActions;
  //!
  std::set<IAction*> _actionsFinished;

};