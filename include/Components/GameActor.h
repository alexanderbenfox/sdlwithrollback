#pragma once

#include "IComponent.h"
#include "Geometry.h"

//all of the possible states for animation??
enum class AerialState
{
  GROUNDED, JUMPING, FALLING
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
  GameActor(std::shared_ptr<Entity> entity) :
    _controllableState(true), IComponent(entity) {}
  //!
  virtual void Update(float dt) override;
  //!
  virtual void HandleMovementCommand(Vector2<float> movement);
  //!
  virtual void HandleJabButtonCommand();

protected:
  //!
  const float _baseSpeed = 300.0f;
  //!
  bool _controllableState;

  //!
  AerialState _aerial;
  MovingState _moving;
};
