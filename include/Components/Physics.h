#pragma once
#include "Collider.h"

class UniversalPhysicsSettings : public IDebuggable
{
public:
  static UniversalPhysicsSettings& Get()
  {
    static UniversalPhysicsSettings instance;
    return instance;
  }

  float Gravity = 2700.0f;
  float JumpVelocity = 1200.0f;

  virtual void ParseCommand(const std::string& command) override
  {
    auto split = StringUtils::Split(command, ' ');

    float value = std::stof(split[1]);

    if (split[0] == "gravity")
    {
      Gravity = value;
    }
    else if (split[0] == "jumpvelocity")
    {
      JumpVelocity = value;
    }
  }

  virtual std::string GetIdentifier() override { return "physics"; }

private:
  UniversalPhysicsSettings() = default;

};

struct DynamicCollision
{
  std::shared_ptr<RectColliderD> collider;
  std::shared_ptr<RectColliderD> collided;
  Vector2<double> movement;
};

static std::vector<DynamicCollision> dynamicCollisionsThisFrame;

//!
class Physics : public IComponent
{
public:
  //!
  Physics(std::shared_ptr<Entity> entity) : IComponent(entity) {}
  //!
  virtual void Update(float dt) override;
  //!
  Vector2<double> DoElasticCollisions(const Vector2<double>& movementVector);

  //!
  static Vector2<double> CreateResolveCollisionVector(OverlapInfo<double> overlap, const Vector2<double>& movementVector);

  CollisionSide GetLastCollisionSides() { return _lastCollisionSide; }

  void ApplyVelocity(Vector2<float> vel) { _vel = vel; }

  void ChangeXVelocity(float x) { _vel.x = x; }
  void ChangeYVelocity(float y) { _vel.y = y; }

  const Vector2<float>& GetVelocity() { return _vel; }

  friend std::ostream& operator<<(std::ostream& os, const Physics& phys);
  friend std::istream& operator>>(std::istream& is, Physics& phys);

private:
  CollisionSide _lastCollisionSide;
  //!
  Vector2<float> _vel;
  //!
  Vector2<float> _acc;

};

template <> void ComponentManager<Physics>::PreUpdate();
template <> void ComponentManager<Physics>::PostUpdate();
