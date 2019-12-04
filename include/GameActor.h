#pragma once
#include "Entity.h"

class Physics : public IComponent
{
public:
  virtual void Update(float dt) override
  {
    _owner->transform.position += (_vel * dt + _acc * dt * dt);
  }

private:
  Vector2<int> _vel;
  Vector2<int> _acc;
};

class GameActor : public IComponent
{
public:
  GameActor(Entity* entity) : IComponent(entity)
  {
    //AddComponent<Physics>();
    //_physics = GetComponent<Physics>();
  }

  virtual void Update(float dt) override
  {
    Vector2<int> mVector = _vel * dt;
    _owner->transform.position += mVector;
  }

  virtual void HandleMovementCommand(Vector2<int> movement)
  {
    _vel = _baseSpeed * movement;
  }

  Physics* GetPhysics() { return _physics; }

protected:
  Physics* _physics;
  Vector2<int> _vel;

  const int _baseSpeed = 300;
};