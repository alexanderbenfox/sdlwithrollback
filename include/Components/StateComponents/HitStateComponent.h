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
  int GetRemainingFrames();

private:
  //!
  ActionTimer* _linkedTimer;
};
