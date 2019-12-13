#pragma once
#include "Collider.h"

const float Gravity = 300.0f;

enum class CollisionSide : unsigned char
{
  NONE = 0x00,
  UP = 0x01,
  DOWN = 0x02,
  RIGHT = 0x04,
  LEFT = 0x08
};

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