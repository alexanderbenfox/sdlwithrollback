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

  virtual std::string GetIdentifier() { return "physics"; }

  float Gravity = 2700.0f;
  float JumpVelocity = 920.0f;

private:
  UniversalPhysicsSettings() = default;

};

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

  CollisionSide GetLastCollisionSides() { return _lastCollisionSide; }

  void ApplyVelocity(Vector2<float> vel) { _vel = vel; }

  void ChangeXVelocity(float x) { _vel.x = x; }
  void ChangeYVelocity(float y) { _vel.y = y; }

  const Vector2<float>& GetVelocity() { return _vel; }

private:
  CollisionSide _lastCollisionSide;
  //!
  Vector2<float> _vel;
  //!
  Vector2<float> _acc;

};