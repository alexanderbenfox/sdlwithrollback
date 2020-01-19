#include "Components/Physics.h"

//______________________________________________________________________________
void Physics::Update(float dt)
{
  _acc = Vector2<float>(0, UniversalPhysicsSettings::Get().Gravity);
  if (auto rCollider = _owner->GetComponent<RectColliderD>())
  {
    if (rCollider->IsStatic()) _acc = Vector2<float>(0, 0);
  }
  // Apply last frame of acceleration to the velocity
  _vel += _acc * dt;

  // Create the movement vector based on speed and acceleration of the object
  auto fix = [](float dt) { return (int)std::floor(10000 * dt) / 10000.0; };

  Vector2<double> movementVector = Vector2<double>(_vel.x * fix(dt), _vel.y * fix(dt));
  // Check collisions with other physics objects here and correct the movement vector based on those collisions
  Vector2<double> collisionAdjustmentVector = DoElasticCollisions(movementVector);

  Vector2<float> caVelocity = (Vector2<float>((float)(collisionAdjustmentVector.x) / (float)fix(dt), (float)(collisionAdjustmentVector.y) / (float)fix(dt)));
  // Convert adjustment vector to a velocity and change object's velocity based on the adjustment
  _vel += caVelocity;

  // Add the movement vector to the entityd
  _owner->transform.position += _vel * dt;

}

//______________________________________________________________________________
Vector2<double> Physics::DoElasticCollisions(const Vector2<double>& movementVector)
{
  OverlapInfo<double> correction;

  auto myCollider = _owner->GetComponent<RectColliderD>();
  if (myCollider && !myCollider->IsStatic())
  {
    Rect<double> potentialRect = myCollider->rect;

    potentialRect = Rect<double>(Vector2<double>(_owner->transform.position.x, _owner->transform.position.y),
      Vector2<double>(_owner->transform.position.x + potentialRect.Width(),
        _owner->transform.position.y + potentialRect.Height()));

    potentialRect.Move(movementVector + correction.amount);

    for (auto collider : ComponentManager<RectColliderD>::Get().All())
    {
      if (myCollider != collider)
      {
        if (potentialRect.Collides(collider->rect))
        {
          // only check right or left on dynamic colliders
          if(!collider->IsStatic())
          {
            dynamicCollisionsThisFrame.push_back({myCollider, collider, movementVector});
          }
          else
          {
            // return the reverse of the overlap to correct for the collision
            auto overlap = potentialRect.Overlap(collider->rect);
            correction.collisionSides |= overlap.collisionSides;
            correction.amount += CreateResolveCollisionVector(overlap, movementVector);
          }
        }
      }
    }
  }

  _lastCollisionSide = correction.collisionSides;
  return correction.amount;
}

Vector2<double> Physics::CreateResolveCollisionVector(OverlapInfo<double> overlap, const Vector2<double>& movementVector)
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

template <> void ComponentManager<Physics>::PreUpdate()
{
  dynamicCollisionsThisFrame.clear();
}

template <> void ComponentManager<Physics>::PostUpdate()
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
        auto pushAmount_collided = GetPushAmount(collider, collided, dynamicCollisionsThisFrame[i].movement, -0.5);
        auto pushAmount_collider = GetPushAmount(collided, collider, dynamicCollisionsThisFrame[twoWayCollisionIndex].movement, -0.5);

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
