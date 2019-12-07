#pragma once
#include "Entity.h"
#include "Collider.h"
#include <cassert>

class Physics : public IComponent
{
public:
  Physics(std::shared_ptr<Entity> entity) : IComponent(entity) {}

  virtual void Update(float dt) override
  {
    Vector2<float> movementVector = _vel * dt + _acc * (dt * dt / 2.0f);
    movementVector += DoElasticCollisions(movementVector);
    _owner->transform.position += movementVector;
    _vel += _acc * dt;
  }

  Vector2<float> DoElasticCollisions(const Vector2<float>& movementVector)
  {
    auto myCollider = _owner->GetComponent<RectCollider>();
    if (myCollider && !myCollider->IsStatic())
    {
      Rect<float> potentialRect = myCollider->rect;
      potentialRect.Move(movementVector);

      for (auto collider : ComponentManager<RectCollider>::Get().All())
      {
        if (myCollider != collider)
        {
          if (potentialRect.Collides(collider->rect))
          {
            //! return the reverse of the overlap to correct for the collision
            return -1.01f * potentialRect.Overlap(collider->rect, movementVector);
          }
        }
      }
    }
    return Vector2<float>(0.0f, 0.0f);
  }

  Vector2<float> _vel;
  Vector2<float> _acc;
};

class GameActor : public IComponent
{
public:
  GameActor(std::shared_ptr<Entity> entity) : IComponent(entity) {}

  virtual void Update(float dt) override {}

  virtual void HandleMovementCommand(Vector2<float> movement)
  {
    auto vel = _baseSpeed * movement;
    assert(_owner->GetComponent<Physics>());
    _owner->GetComponent<Physics>()->_vel = vel;
  }

protected:
  const float _baseSpeed = 300.0f;
};