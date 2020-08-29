#pragma once
#include "Core/ECS/IComponent.h"
#include "Components/ActionComponents.h"
#include "Systems/TimerSystem/ActionTimer.h"

struct HitStateComponent : public IComponent
{
  void OnRemove(const EntityID& entity) override;
};

//! Attach to entity when it is in the "being thrown" state
class GrappledStateComponent : public HitStateComponent
{
public:
  //!
  GrappledStateComponent() : HitStateComponent() {}

};
