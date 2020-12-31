#pragma once
#include <string>
#include "Core/Interfaces/Serializable.h"

// describe the kind of blocking that can be done for this attack
enum class HitType
{
  Low, Mid, High
};

static std::string ToString(HitType type)
{
  switch (type)
  {
  case HitType::Low:
    return "Low";
  case HitType::Mid:
    return "Mid";
  case HitType::High:
    return "High";
  default:
    return "Mid";
  }
}

static std::string ToString(HitType&& type)
{
  switch (type)
  {
  case HitType::Low:
    return "Low";
  case HitType::Mid:
    return "Mid";
  case HitType::High:
    return "High";
  default:
    return "Mid";
  }
}

static HitType FromString(const std::string& str)
{
  if (str == "Low")
    return HitType::Low;
  else if (str == "Mid")
    return HitType::Mid;
  else
    return HitType::High;
}
