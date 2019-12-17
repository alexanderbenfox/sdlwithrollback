#pragma once
#include "Collider.h"

const float Gravity = 1000.0f;

//!
class Physics : public IComponent
{
public:
  //!
  Physics(std::shared_ptr<Entity> entity) : IComponent(entity) {}
  //!
  virtual void Update(float dt) override;
  //!
  Vector2<double> DoElasticCollisions(const Vector2<double>& movementVector);
  //!
  Vector2<float> _vel;
  //!
  Vector2<float> _acc;

  CollisionSide GetLastCollisionSides() { return _lastCollisionSide; }

private:
  CollisionSide _lastCollisionSide;
};