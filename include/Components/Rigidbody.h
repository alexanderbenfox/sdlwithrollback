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

//!
class Rigidbody : public IComponent
{
public:
  //!
  Rigidbody(std::shared_ptr<Entity> entity) : _useGravity(false), IComponent(entity) {}
  //!
  void Init(bool useGravity)
  {
    _useGravity = useGravity;
    _addedAccel = UniversalPhysicsSettings::Get().Gravity;
  }

  CollisionSide _lastCollisionSide;
  //!
  Vector2<float> _vel;
  //!
  Vector2<float> _acc;
  //!
  float _addedAccel;
  //!
  bool _useGravity;

  friend std::ostream& operator<<(std::ostream& os, const Rigidbody& rb);
  friend std::istream& operator>>(std::istream& is, Rigidbody& rb);

};

template <> struct ComponentTraits<Rigidbody>
{
  static const uint64_t GetSignature() { return 1 << 2;}
};