#pragma once

#include "IComponent.h"
#include "Geometry.h"

enum class StanceState
{
  CROUCHING, STANDING, JUMPING, FALLING
};

enum class MovingState
{
  FORWARD, BACKWARD, IDLE
};

//!
class GameActor : public IComponent
{
public:
  //!
  GameActor(std::shared_ptr<Entity> entity) : _controllableState(true), IComponent(entity) {}
  //!
  virtual void Update(float dt) override;
  //!
  virtual void HandleMovementCommand(Vector2<float> movement);
  //!
  virtual void HandleLightButtonCommand();
  //!
  virtual void HandleStrongButtonCommand();
  //!
  virtual void HandleHeavyButtonCommand();

  void SetStance(StanceState state) { _stance = state; }

protected:
  //!
  const float _baseSpeed = 300.0f;
  //!
  bool _controllableState;

  //!
  StanceState _stance;
  MovingState _moving;
};
