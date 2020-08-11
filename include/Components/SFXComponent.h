#pragma once
#include "Components/IComponent.h"

#include "Core/Math/Vector2.h"
#include "ActionTimer.h"

class SFXComponent : public IComponent
{
public:
  SFXComponent(std::shared_ptr<Entity> entity);
  ~SFXComponent();
  void ShowHitSparks();
  void ShowBlockSparks();

  Vector2<float> showLocation;

private:
  std::shared_ptr<Entity> _sfxEntity;
  std::shared_ptr<ActionTimer> _subroutine;

};
