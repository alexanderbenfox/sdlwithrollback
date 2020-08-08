#pragma once
#include "Components/IComponent.h"
#include "StateMachine/ActionTimer.h"

class HitStateComponent : public IComponent
{
public:
  //!
  HitStateComponent(std::shared_ptr<Entity> owner);
  //!
  virtual ~HitStateComponent() override;
  //!
  void SetTimer(ActionTimer* timer);
  //!
  virtual int GetRemainingFrames();

protected:
  //!
  ActionTimer* _linkedTimer;
};

//! Attach to entity when it is in the "being thrown" state
class GrappledStateComponent : public HitStateComponent
{
public:
  //!
  GrappledStateComponent(std::shared_ptr<Entity> owner) : HitStateComponent(owner) {}

};
