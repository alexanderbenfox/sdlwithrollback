#include "Systems/Physics.h"

void PhysicsSystem::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    auto collidingObj = tuple.second;
    // try moving
    // Apply last frame of acceleration to the velocity

    auto rigidbody = std::get<Rigidbody*>(collidingObj);

    if (!rigidbody->enabled)
      continue;

    auto collider = std::get<DynamicCollider*>(collidingObj);
    auto transform = std::get<Transform*>(collidingObj);

    Vector2<float>& vel = rigidbody->_vel;
    Vector2<float>& acc = rigidbody->_acc;
    float& addedAcc = rigidbody->_addedAccel;

    vel += (acc + Vector2<float>(0, addedAcc)) * dt;

    // Create the movement vector based on speed and acceleration of the object
    double ddt = ToDouble(dt);

    Vector2<double> movementVector = Vector2<double>(vel.x * ddt, vel.y * ddt);

    // Check collisions with other physics objects here and correct the movement vector based on those collisions

    // corrections that will be added to the momentum of the collider
    OverlapInfo<double> futureCorrection;
    // corrections that will be added to the position for this frame only
    OverlapInfo<double> currentCorrection;

    // loop over each other collider
    // if in hitstun, do elastic collision
    AdjustMovementForCollisions(collider, movementVector, futureCorrection, currentCorrection, rigidbody->elasticCollisions, rigidbody->ignoreDynamicColliders);

    Vector2<float> caVelocity = PositionAdjustmentToVelocity(futureCorrection.amount, ddt);
    // Convert adjustment vector to a velocity and change object's velocity based on the adjustment
    vel += caVelocity;
    //
    Vector2<float> instVelocity = PositionAdjustmentToVelocity(currentCorrection.amount, ddt);
    // Add the movement vector to the entityd
    transform->position += (vel + instVelocity) * dt;

    // end of frame, change the collision sides for this frame
    rigidbody->_lastCollisionSide = futureCorrection.collisionSides;
  }
}

Vector2<double> PhysicsSystem::CreateResolveCollisionVector(OverlapInfo<double>& overlap, const Vector2<double>& movementVector)
{
  Vector2<double> resolutionVector(0.0, 0.0);
  //! Do each individually
  if ((overlap.collisionSides & CollisionSide::LEFT) != CollisionSide::NONE ||
    (overlap.collisionSides & CollisionSide::RIGHT) != CollisionSide::NONE)
  {
    resolutionVector.x = -overlap.amount.x;
  }
  if ((overlap.collisionSides & CollisionSide::UP) != CollisionSide::NONE ||
    (overlap.collisionSides & CollisionSide::DOWN) != CollisionSide::NONE)
  {
    resolutionVector.y = -overlap.amount.y;
  }

  // in the case that we're hitting a corner, only correct by the value that has the greater amount of overlap
  if (overlap.numCollisionSides > 1)
  {
    if (std::abs(overlap.amount.x) > std::abs(overlap.amount.y))
    {
      resolutionVector.x = 0;
      overlap.collisionSides &= ~CollisionSide::LEFT;
      overlap.collisionSides &= ~CollisionSide::RIGHT;
    }
    else
    {
      resolutionVector.y = 0;
      overlap.collisionSides &= ~CollisionSide::DOWN;
      overlap.collisionSides &= ~CollisionSide::UP;
    }

    // if movement isnt in the same "direction" as the movement vector, dont adjust in that direction
    if ((overlap.amount.x > 0 && movementVector.x <= 0) || (overlap.amount.x < 0 && movementVector.x >= 0))
    {
      resolutionVector.x = 0;
    }
    if ((overlap.amount.y > 0 && movementVector.y < 0) || (overlap.amount.y < 0 && movementVector.y > 0))
    {
      resolutionVector.y = 0;
    }
  }
  return resolutionVector;
}

double PhysicsSystem::ToDouble(const float& f)
{
  return (int)std::floor(10000 * f) / 10000.0;
}

Vector2<float> PhysicsSystem::PositionAdjustmentToVelocity(const Vector2<double>& overlap, const double& ddt)
{
  if (ddt == 0.0)
    return Vector2<float>::Zero;
  return (Vector2<float>((float)(overlap.x) / (float)ddt, (float)(overlap.y) / (float)ddt));
}

OverlapInfo<double> PhysicsSystem::GetPushOnDynamicCollision(Rect<double>& collider, Rect<double>& collided, const Vector2<double> movement, double pushFactor)
{
  OverlapInfo<double> overlap = RectHelper::Overlap(collider, collided);

  overlap.amount.y = 0;
  overlap.collisionSides &= ~CollisionSide::UP;
  overlap.collisionSides &= ~CollisionSide::DOWN;
  if(overlap.numCollisionSides > 0)
    overlap.numCollisionSides = 1;

  overlap.amount = CreateResolveCollisionVector(overlap, movement);
  overlap.amount.x *= pushFactor;
  return overlap;
}

void PhysicsSystem::AdjustMovementForCollisions( RectColliderD* colliderComponent, const Vector2<double>& movementVector, OverlapInfo<double>& momentum, OverlapInfo<double>& inst, bool elastic, bool ignoreDynamic)
{
  Rect<double> potentialRect = colliderComponent->rect;
  potentialRect.MoveRelative(movementVector);

  // process all dynamic collisions
  if (!ignoreDynamic)
  {
    for (auto otherCollider : ComponentManager<DynamicCollider>::Get().All())
    {
      if (otherCollider.get() != colliderComponent && potentialRect.Intersects(otherCollider->rect))
      {
        // only check right or left on dynamic colliders
        auto push = GetPushOnDynamicCollision(colliderComponent->rect, otherCollider->rect, movementVector, 0.5);

        // add to instantaneous corrections because we dont want to maintain momentum for these kinds of collisions
        inst.collisionSides |= push.collisionSides;
        inst.amount += push.amount;
      }
    }
  }
  // process static collisions
  for (auto otherCollider : ComponentManager<StaticCollider>::Get().All())
  {
    if (potentialRect.Intersects(otherCollider->rect))
    {
      // return the reverse of the overlap to correct for the collision
      auto overlap = RectHelper::Overlap(potentialRect, otherCollider->rect);

      if(colliderComponent->rect.Intersects(otherCollider->rect))
      {
        inst.amount += CreateResolveCollisionVector(overlap, movementVector);
        momentum.collisionSides |= overlap.collisionSides;
      }
      else
      {
        // if elastic collision, rather than just try and stop the collision, actually bounce it back in the opposite direction
        if(!elastic)
          momentum.amount += CreateResolveCollisionVector(overlap, movementVector);
        else
          momentum.amount += 2.0 * CreateResolveCollisionVector(overlap, movementVector);

        momentum.collisionSides |= overlap.collisionSides;
      }
    }
  }
}
