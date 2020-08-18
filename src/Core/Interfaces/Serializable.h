#pragma once
#include <ostream>
#include <istream>
#include <vector>

//! Serialized buffer
typedef std::vector<char> SBuffer;

struct ISerializable
{
  virtual void Serialize(std::ostream& os) const = 0;
  virtual void Deserialize(std::istream& is) = 0;
};

//! Specialized serializer for writing primitive types to char* buffer
template <typename T>
struct Serializer
{
  static void Serialize(std::ostream& os, const T& item) { os.write((const char*)&item, sizeof(T)); }
  static void Deserialize(std::istream& is, T& item) { is.read((char*)&item, sizeof(T)); }
};
