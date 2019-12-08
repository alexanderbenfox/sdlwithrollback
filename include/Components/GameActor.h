#pragma once
#include "IComponent.h"
#include "Geometry.h"

//!
class GameActor : public IComponent
{
public:
  //!
  GameActor(std::shared_ptr<Entity> entity) :
    _controllableState(true), IComponent(entity) {}
  //!
  virtual void Update(float dt) override {}
  //!
  virtual void HandleMovementCommand(Vector2<float> movement);

protected:
  //!
  const float _baseSpeed = 300.0f;
  //!
  bool _controllableState;

};
