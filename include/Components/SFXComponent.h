#pragma once
#include "Components/IComponent.h"

#include "Core/Math/Vector2.h"
#include "ActionTimer.h"

class SFXComponent : public IComponent
{
public:
  void OnAdd(const EntityID& entity) override;
  void OnRemove(const EntityID& entity) override;

  void ShowHitSparks(bool directionRight);
  void ShowBlockSparks(bool directionRight);

  Vector2<float> showLocation;

private:
  std::shared_ptr<Entity> _sfxEntity;
  std::shared_ptr<ActionTimer> _subroutine;

};
