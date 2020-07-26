#pragma once
// will be used later when trying to adjust this to a more
// pure ECS system based on this:
// https://www.gdcvault.com/play/1024001/-Overwatch-Gameplay-Architecture-and
#include "Systems/ISystem.h"
#include "Components/Collider.h"
#include "Components/Rigidbody.h"
#include "Components/Transform.h"
#include "Components/GameActor.h"

class PhysicsSystem : public ISystem<DynamicCollider, Rigidbody, Transform>
{
public:
  friend class Rigidbody;
  static void DoTick(float dt);

  const static float dragThreshold;

private:
  static Vector2<double> CreateResolveCollisionVector(OverlapInfo<double>& overlap, const Vector2<double>& movementVector);
  static double ToDouble(const float& f);
  static Vector2<float> PositionAdjustmentToVelocity(const Vector2<double>& overlap, const double& ddt);
  static OverlapInfo<double> GetPushOnDynamicCollision(Rect<double>& collider, Rect<double>& collided, const Vector2<double> movement, double pushFactor);
  static void AdjustMovementForCollisions(RectColliderD* colliderComponent, const Vector2<double>& movementVector, OverlapInfo<double>& momentum, OverlapInfo<double>& inst, bool elastic, bool ignoreDynamic);
};

struct PushComponent : public IComponent
{
  PushComponent(std::shared_ptr<Entity> entity) : IComponent(entity) {}
  float pushAmount;
  float amountPushed = 0.0f;
  float velocity;
  bool init = false;
};

class PushSystem : public ISystem<Rigidbody, PushComponent, Transform>
{
public:
  static void DoTick(float dt)
  {
    std::vector<Transform*> deleteList;

    for (auto tuple : Tuples)
    {
      Rigidbody* rigidbody = std::get<Rigidbody*>(tuple.second);
      PushComponent* push = std::get<PushComponent*>(tuple.second);
      Transform* transform = std::get<Transform*>(tuple.second);

      if (!push->init)
      {
        rigidbody->_vel.x = push->velocity;
        push->init = true;
      }
      
      push->amountPushed += push->velocity * dt;
      if (std::fabs(push->amountPushed) >= std::fabs(push->pushAmount))
      {
        rigidbody->_vel.x -= push->velocity;
        deleteList.push_back(transform);
      }
    }

    for (Transform* transform : deleteList)
      transform->RemoveComponent<PushComponent>();
    deleteList.clear();

  }
};