#pragma once
#include "Core/ECS/IComponent.h"
#include "Core/ECS/Entity.h"

#include "Core/Math/Vector2.h"
#include "Systems/TimerSystem/ActionTimer.h"

class SFXComponent : public IComponent
{
public:
  Vector2<float> showLocation;
  EntityID sfxEntityID = 0;
  bool needsInit = true;

  enum class Request { None, HitSparks, BlockSparks };
  Request pending = Request::None;
  bool pendingDirectionRight = false;

  // Timer reference for cancellation — managed by SFXSystem
  std::shared_ptr<ActionTimer> subroutine;
};
