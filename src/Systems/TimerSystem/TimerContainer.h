#pragma once
#include "Core/ECS/IComponent.h"
#include "Systems/TimerSystem/ActionTimer.h"

struct TimerContainer : public IComponent
{
  void OnRemove(const EntityID& entity) override
  {
    for (auto timing : timings)
      timing->OnComplete();
  }
  std::vector<std::shared_ptr<ActionTimer>> timings;
};
