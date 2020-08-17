#pragma once
#include <ostream>
#include <istream>
#include <vector>

//! Serialized buffer
typedef std::vector<unsigned char> SBuffer;

struct ISerializable
{
  virtual void Serialize(std::ostream& os) const = 0;
  virtual void Deserialize(std::istream& is) = 0;
};
