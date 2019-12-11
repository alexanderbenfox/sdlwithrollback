#pragma once
#include "Collider.h"

const float Gravity = 100.0f;

//!
class Physics : public IComponent
{
public:
  //!
  Physics(std::shared_ptr<Entity> entity) : IComponent(entity) {}
  //!
  virtual void Update(float dt) override;
  //!
  Vector2<float> DoElasticCollisions(const Vector2<float>& movementVector);
  //!
  Vector2<float> _vel;
  //!
  Vector2<float> _acc;
};