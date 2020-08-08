#pragma once
#include "Components/IComponent.h"
#include "StateMachine/ActionTimer.h"

struct TimerContainer : public IComponent
{
  TimerContainer(std::shared_ptr<Entity> owner) : IComponent(owner) {}
  virtual ~TimerContainer()
  {
    for (auto timing : timings)
      timing->OnComplete();
  }
  std::vector<std::shared_ptr<ActionTimer>> timings;
};
