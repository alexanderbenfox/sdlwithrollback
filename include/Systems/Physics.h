#pragma once
// will be used later when trying to adjust this to a more
// pure ECS system based on this:
// https://www.gdcvault.com/play/1024001/-Overwatch-Gameplay-Architecture-and
#include "Systems/ISystem.h"
#include "Components/Collider.h"
#include "Components/Physics.h"
#include "GameManagement.h"
#include "Components/Transform.h"

struct CollidableTuples
{
  RectColliderD* collider;
  Physics* phys;
  Transform* transform;
};

class PhysicsSystem : public ISystem<RectColliderD, Physics, Transform>
{
public:
  friend class Physics;
  // responsible for setting 
  static void DoTick(float dt);

private:
  static Vector2<double> DoElasticCollisions(const Vector2<double>& movementVector);
  static Vector2<double> CreateResolveCollisionVector(OverlapInfo<double> overlap, const Vector2<double>& movementVector);
  static void PreUpdate();
  static void PostUpdate();

};

inline void PhysicsSystem::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    auto collidingObj = tuple.second;
    // try moving
    // Apply last frame of acceleration to the velocity

    auto physicsComponent = std::get<Physics*>(collidingObj);
    auto rectColliderComponent = std::get<RectColliderD*>(collidingObj);
    auto transformComponent = std::get<Transform*>(collidingObj);

    Vector2<float>& vel = physicsComponent->_vel;
    Vector2<float>& acc = physicsComponent->_acc;
    float& addedAcc = physicsComponent->_addedAccel;

    vel += (acc + Vector2<float>(0, addedAcc)) * dt;

    // Create the movement vector based on speed and acceleration of the object
    auto fix = [](float dt) { return (int)std::floor(10000 * dt) / 10000.0; };

    Vector2<double> movementVector = Vector2<double>(vel.x * fix(dt), vel.y * fix(dt));

    Rect<double> potentialRect = rectColliderComponent->rect;
    potentialRect = Rect<double>(Vector2<double>(transformComponent->position.x, transformComponent->position.y),
      Vector2<double>(transformComponent->position.x + potentialRect.Width(),
        transformComponent->position.y + potentialRect.Height()));
    potentialRect.Move(movementVector);

    // Check collisions with other physics objects here and correct the movement vector based on those collisions
    OverlapInfo<double> correction;

    for (auto otherCollider : ComponentManager<RectColliderD>::Get().All())
    {
      if (potentialRect.Collides(otherCollider->rect))
      {
        // only check right or left on dynamic colliders
        if(!otherCollider->IsStatic())
        {
          dynamicCollisionsThisFrame.push_back(
            {std::shared_ptr<RectColliderD>(rectColliderComponent),
            std::shared_ptr<RectColliderD>(otherCollider),
            movementVector});
        }
        else
        {
          // return the reverse of the overlap to correct for the collision
          auto overlap = potentialRect.Overlap(otherCollider->rect);
          correction.collisionSides |= overlap.collisionSides;
          correction.amount += CreateResolveCollisionVector(overlap, movementVector);
        }
      }
    }
    physicsComponent->_lastCollisionSide = correction.collisionSides;

    Vector2<float> caVelocity = (Vector2<float>((float)(correction.amount.x) / (float)fix(dt), (float)(correction.amount.y) / (float)fix(dt)));
    // Convert adjustment vector to a velocity and change object's velocity based on the adjustment
    vel += caVelocity;

    // Add the movement vector to the entityd
    transformComponent->position += vel * dt;
  }
}

inline Vector2<double> PhysicsSystem::CreateResolveCollisionVector(OverlapInfo<double> overlap, const Vector2<double>& movementVector)
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
    }
    else
    {
      resolutionVector.y = 0;
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
};

inline void PhysicsSystem::PreUpdate()
{
  dynamicCollisionsThisFrame.clear();
}

inline void PhysicsSystem::PostUpdate()
{
  for(int i = 0; i < dynamicCollisionsThisFrame.size(); i++)
  {
    bool twoWayCollision = false;
    int twoWayCollisionIndex = -1;
    auto collider = dynamicCollisionsThisFrame[i].collider;
    auto collided = dynamicCollisionsThisFrame[i].collided;

    for(int j = (i+1); j < dynamicCollisionsThisFrame.size(); j++)
    {
      if(collider == dynamicCollisionsThisFrame[j].collided &&
        collided == dynamicCollisionsThisFrame[j].collider)
      {
        twoWayCollision = true;
        twoWayCollisionIndex = j;
        break;
      }
    }

    auto GetPushAmount = [](
      std::shared_ptr<RectColliderD> collider,
      std::shared_ptr<RectColliderD> collided,
      const Vector2<double> movement,
      double pushFactor)
    {
        auto overlap = collider->rect.Overlap(collided->rect);

        overlap.amount.y = 0;
        overlap.collisionSides &= ~CollisionSide::UP;
        overlap.collisionSides &= ~CollisionSide::DOWN;
        if(overlap.numCollisionSides > 0)
          overlap.numCollisionSides = 1;

        Vector2<double> pushAmount = Physics::CreateResolveCollisionVector(overlap, movement);
        pushAmount.x *= pushFactor;
        return pushAmount;
    };

    if(twoWayCollision)
    {
      //check if they are still colliding
      if(collider->rect.Collides(collided->rect))
      {
        auto pushAmount_collided = GetPushAmount(collider, collided, dynamicCollisionsThisFrame[i].movement, -0.25);
        auto pushAmount_collider = GetPushAmount(collided, collider, dynamicCollisionsThisFrame[twoWayCollisionIndex].movement, -0.25);

        collided->MoveUnit(pushAmount_collided);
        collider->MoveUnit(pushAmount_collider);
      }
      dynamicCollisionsThisFrame.erase(dynamicCollisionsThisFrame.begin() + twoWayCollisionIndex);
    }
    else
    {
      if(collider->rect.Collides(collided->rect))
      {
        collided->MoveUnit(GetPushAmount(collider, collided, dynamicCollisionsThisFrame[i].movement, -1.0));
      }
    }
  }
}