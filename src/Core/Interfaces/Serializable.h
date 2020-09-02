#pragma once
#include <ostream>
#include <istream>
#include <vector>
#include <sstream>

//! Serialized buffer
typedef std::vector<char> SBuffer;

struct ISerializable
{
  virtual void Serialize(std::ostream& os) const = 0;
  virtual void Deserialize(std::istream& is) = 0;

  virtual std::string Log() = 0;
};

//! Specialized serializer for writing primitive types to char* buffer
template <typename T>
struct Serializer
{
  static void Serialize(std::ostream& os, const T& item) { os.write((const char*)&item, sizeof(T)); }
  static void Deserialize(std::istream& is, T& item) { is.read((char*)&item, sizeof(T)); }
};

template <> struct Serializer<std::string>
{
  static void Serialize(std::ostream& os, const std::string& item)
  {
    // write out string size first
    int strSize = item.size();
    Serializer<int>::Serialize(os, strSize);

    // write out the actual contents of the string
    os.write((const char*)item.data(), strSize);
  }

  static void Deserialize(std::istream& is, std::string& item)
  {
    // read in string size first
    int strSize = 0;
    Serializer<int>::Deserialize(is, strSize);

    // resize before reading in
    item.resize(strSize, '\000');
    is.read(item.data(), strSize);

  }
};
