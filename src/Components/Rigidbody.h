#pragma once
#include "Collider.h"
#include "Core/Geometry2D/RectHelper.h"

//!
struct DynamicCollider : public RectColliderD {};

//!
struct StaticCollider : public RectColliderD {};

//! Empty gravity component
struct Gravity : public IComponent, public ISerializable
{
  Vector2<float> force;

  void Serialize(std::ostream& os) const override { os << force; }
  void Deserialize(std::istream& is) override { is >> force; }
};

//!
struct Rigidbody : public IComponent, public ISerializable
{
public:
  //!
  Rigidbody() : lastCollisionSide(CollisionSide::NONE), elasticCollisions(false), ignoreDynamicColliders(false), IComponent() {}
  //! Last side(s) on physics collider that collided with another collider
  CollisionSide lastCollisionSide;
  //! Current velocity on rigidbody
  Vector2<float> velocity;
  //! Current acceleration on rigidbody
  Vector2<float> acceleration;

  //! Should collisions on this bounce or be rigid
  bool elasticCollisions;
  //! Should collision checks include other dynamic colliders
  bool ignoreDynamicColliders;

  //! Helper fns
  bool IsGrounded() const { return HasState(lastCollisionSide, CollisionSide::DOWN); }

  void Serialize(std::ostream& os) const override;
  void Deserialize(std::istream& is) override;

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
  static void Init(Rigidbody& component, const ComponentInitParams<Rigidbody>& params)
  {
    component.velocity = params.velocity;
  }
};

template <> struct ComponentInitParams<Gravity>
{
  Vector2<float> force;
  static void Init(Gravity& component, const ComponentInitParams<Gravity>& params)
  {
    component.force = params.force;
  }
};
