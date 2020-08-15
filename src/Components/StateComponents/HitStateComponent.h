#pragma once
#include "Core/ECS/IComponent.h"
#include "Components/ActionComponents.h"
#include "ActionTimer.h"

class HitStateComponent : public IComponent
{
public:
  HitStateComponent();
  //!
  void OnRemove(const EntityID& entity) override;
  //!
  void SetTimer(TimedActionComponent* timer);
  //!
  virtual int GetRemainingFrames();

protected:
  //!
  TimedActionComponent* _linkedTimer;
};

//! Attach to entity when it is in the "being thrown" state
class GrappledStateComponent : public HitStateComponent
{
public:
  //!
  GrappledStateComponent() : HitStateComponent() {}

};
