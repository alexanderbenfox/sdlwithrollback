#pragma once
#include "Core/ECS/IComponent.h"

#include "Components/Rigidbody.h"
#include "Components/StateComponent.h"

#include <sstream>

struct AttackActionComponent : public IComponent, ISerializable
{
  ActionState type = ActionState::NONE;

  void Serialize(std::ostream& os) const override
  {
    Serializer<ActionState>::Serialize(os, type);
  }
  void Deserialize(std::istream& is) override
  {
    Serializer<ActionState>::Deserialize(is, type);
  }

  std::string Log() override
  {
    std::stringstream ss;
    ss << "AttackActionComponent: \n";
    ss << "\tAction type: " << (int)type << "\n";
    return ss.str();
  }
};

struct GrappleActionComponent : public IComponent {};

struct ReceivedGrappleAction : public IComponent, ISerializable
{
  int damageAmount;
  bool isKillingBlow = false;
  int damageAndKnockbackDelay = 0;

  void Serialize(std::ostream& os) const override
  {
    Serializer<int>::Serialize(os, damageAmount);
    Serializer<bool>::Serialize(os, isKillingBlow);
    Serializer<int>::Serialize(os, damageAndKnockbackDelay);
  }
  void Deserialize(std::istream& is) override
  {
    Serializer<int>::Deserialize(is, damageAmount);
    Serializer<bool>::Deserialize(is, isKillingBlow);
    Serializer<int>::Deserialize(is, damageAndKnockbackDelay);
  }

  std::string Log() override
  {
    std::stringstream ss;
    ss << "ReceivedGrappleAction: \n";
    ss << "\tDamage Amount: " << damageAmount << "\n";
    ss << "\tIs Killing blow: " << isKillingBlow << "\n";
    ss << "\tDamage and knockback delay: " << damageAndKnockbackDelay << "\n";
    return ss.str();
  }
};

struct WallPushComponent : public IComponent, ISerializable
{
  float pushAmount = 0.0f;
  float amountPushed = 0.0f;
  float velocity = 0.0f;

  void Serialize(std::ostream& os) const override
  {
    Serializer<float>::Serialize(os, pushAmount);
    Serializer<float>::Serialize(os, amountPushed);
    Serializer<float>::Serialize(os, velocity);
  }
  void Deserialize(std::istream& is) override
  {
    Serializer<float>::Deserialize(is, pushAmount);
    Serializer<float>::Deserialize(is, amountPushed);
    Serializer<float>::Deserialize(is, velocity);
  }

  std::string Log() override
  {
    std::stringstream ss;
    ss << "WallPushComponent: \n";
    ss << "\tPush amount: " << pushAmount << "\n";
    ss << "\tAmount pushed: " << amountPushed << "\n";
    ss << "\tSpeed: " << velocity << "\n";
    return ss.str();
  }
};
