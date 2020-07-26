#pragma once
#include "Collider.h"
#include "Core/Geometry2D/RectHelper.h"

class UniversalPhysicsSettings
{
public:
  static UniversalPhysicsSettings& Get()
  {
    static UniversalPhysicsSettings instance;
    return instance;
  }

  float Gravity = 2700.0f;
  float JumpVelocity = 1200.0f;

private:
  UniversalPhysicsSettings() = default;

};

//!
struct DynamicCollider : public RectColliderD
{
  DynamicCollider(std::shared_ptr<Entity> entity) : RectColliderD(entity) {}
};

//!
struct StaticCollider : public RectColliderD
{
  StaticCollider(std::shared_ptr<Entity> entity) : RectColliderD(entity) {}
};

//!
class Rigidbody : public IComponent
{
public:
  //!
  Rigidbody(std::shared_ptr<Entity> entity) : _useGravity(false), elasticCollisions(false), IComponent(entity) {}
  //!
  void Init(bool useGravity)
  {
    _useGravity = useGravity;
    if(_useGravity)
      _addedAccel = UniversalPhysicsSettings::Get().Gravity;
  }

  CollisionSide _lastCollisionSide;
  //!
  Vector2<float> _vel;
  //!
  float horizontalDragVelocity = 0.0f;
  //!
  Vector2<float> _acc;
  //!
  float _addedAccel = 0.0f;
  //!
  bool _useGravity;
  //!
  bool elasticCollisions;
  //!
  bool ignoreDynamicColliders = false;

  friend std::ostream& operator<<(std::ostream& os, const Rigidbody& rb);
  friend std::istream& operator>>(std::istream& is, Rigidbody& rb);

};

template <> struct ComponentInitParams<DynamicCollider>
{
  Vector2<float> size;
  static void Init(DynamicCollider& component, const ComponentInitParams<DynamicCollider>& params)
  {
    component.Init(Vector2<double>::Zero, params.size);
  }
};

template <> struct ComponentInitParams<Rigidbody>
{
  Vector2<float> velocity;
  bool useGravity;
  static void Init(Rigidbody& component, const ComponentInitParams<Rigidbody>& params)
  {
    component._vel = params.velocity;
    component.Init(params.useGravity);
  }
};
