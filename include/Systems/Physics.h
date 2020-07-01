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
  // responsible for setting 
  static void DoTick(float dt);

private:
  static Vector2<double> CreateResolveCollisionVector(OverlapInfo<double>& overlap, const Vector2<double>& movementVector);
  static double ToDouble(const float& f);
  static Vector2<float> PositionAdjustmentToVelocity(const Vector2<double>& overlap, const double& ddt);
  static OverlapInfo<double> GetPushOnDynamicCollision(Rect<double>& collider, Rect<double>& collided, const Vector2<double> movement, double pushFactor);
  static void AdjustMovementForCollisions(RectColliderD* colliderComponent, const Vector2<double>& movementVector, OverlapInfo<double>& momentum, OverlapInfo<double>& inst, bool elastic, bool ignoreDynamic);
};