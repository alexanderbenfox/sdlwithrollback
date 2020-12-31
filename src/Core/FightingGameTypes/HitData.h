#pragma once
#include "HitType.h"
#include "Core/Math/Vector2.h"
#include "Core/Interfaces/Serializable.h"

//! Holds the data for one frame of attack that is hitting opponent
struct HitData : ISerializable
{
  // how many frames from the moment you get hit will you be unable to do another action
  int framesInStunBlock, framesInStunHit;
  int activeFrames;
  Vector2<float> knockback;
  int damage;
  bool knockdown = false;
  HitType type = HitType::Mid;

  void Serialize(std::ostream& os) const override
  {
    Serializer<int>::Serialize(os, framesInStunBlock);
    Serializer<int>::Serialize(os, framesInStunHit);
    Serializer<int>::Serialize(os, activeFrames);
    os << knockback;
    Serializer<int>::Serialize(os, damage);
    Serializer<bool>::Serialize(os, knockdown);
    Serializer<HitType>::Serialize(os, type);
  }

  void Deserialize(std::istream& is) override
  {
    Serializer<int>::Deserialize(is, framesInStunBlock);
    Serializer<int>::Deserialize(is, framesInStunHit);
    Serializer<int>::Deserialize(is, activeFrames);
    is >> knockback;
    Serializer<int>::Deserialize(is, damage);
    Serializer<bool>::Deserialize(is, knockdown);
    Serializer<HitType>::Deserialize(is, type);
  }

  std::string Log() override
  {
    std::stringstream ss;
    ss << "HitData: \n";
    ss << "\tStun Block :" << framesInStunBlock << " Stun Hit:" << framesInStunHit << " Active Frames:" << activeFrames << "\n";
    ss << "\tKnockback Vector: x=" << knockback.x << " y=" << knockback.y << "\n";
    ss << "\tDamage: " << damage << "\n";
    ss << "\tIs Knockdown? " << knockdown << "\n";
    return ss.str();
  }

};
