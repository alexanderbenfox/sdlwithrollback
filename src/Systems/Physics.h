#pragma once
// will be used later when trying to adjust this to a more
// pure ECS system based on this:
// https://www.gdcvault.com/play/1024001/-Overwatch-Gameplay-Architecture-and
#include "Core/ECS/ISystem.h"
#include "Components/Collider.h"
#include "Components/Rigidbody.h"
#include "Components/Transform.h"
#include "Components/Actors/GameActor.h"

struct ApplyGravitySystem : public ISystem<Rigidbody, Gravity>
{
  static void DoTick(float dt)
  {
    for (const EntityID& entity : Registered)
    {
      Rigidbody& rigidbody = ComponentArray<Rigidbody>::Get().GetComponent(entity);
      Gravity& gravity = ComponentArray<Gravity>::Get().GetComponent(entity);

      rigidbody.velocity += (gravity.force * dt);
    }
  }
};

class PhysicsSystem : public ISystem<DynamicCollider, Rigidbody, Transform>
{
public:
  friend class Rigidbody;
  static void DoTick(float dt);

private:
  static Vector2<double> CreateResolveCollisionVector(OverlapInfo<double>& overlap, const Vector2<double>& movementVector);
  static double ToDouble(const float& f);
  static Vector2<float> PositionAdjustmentToVelocity(const Vector2<double>& overlap, const double& ddt);
  static OverlapInfo<double> GetPushOnDynamicCollision(Rect<double>& collider, Rect<double>& collided, const Vector2<double> movement, double pushFactor);
  static void AdjustMovementForCollisions(RectColliderD* colliderComponent, const Vector2<double>& movementVector, OverlapInfo<double>& momentum, OverlapInfo<double>& inst, bool elastic, bool ignoreDynamic);
};
