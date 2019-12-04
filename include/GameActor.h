#pragma once
#include "Entity.h"

class Physics : public IComponent
{
public:
  virtual void Update(Transform& transform, float dt) override
  {
    transform.position += (_vel * dt + _acc * dt * dt);
  }
  virtual void PushToRenderer(const Transform& transform) override {}

private:
  Vector2<int> _vel;
  Vector2<int> _acc;
};

class GameActor : public IComponent
{
public:
  GameActor()
  {
    //AddComponent<Physics>();
    //_physics = GetComponent<Physics>();
  }

  Physics* GetPhysics() { return _physics; }

protected:
  Physics* _physics;
};