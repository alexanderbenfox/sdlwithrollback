#pragma once
#include <ostream>
#include <istream>

struct ISerializable
{
  virtual void Serialize(std::ostream& os) const = 0;
  virtual void Deserialize(std::istream& is) = 0;
};