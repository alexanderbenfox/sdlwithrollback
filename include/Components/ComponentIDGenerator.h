#pragma once
#include "Globals.h"
#include <bitset>

//______________________________________________________________________________
class ComponentIDGenerator
{
public:
  //! Generate bit flag for component (used on ComponentTraits initialization)
  static std::bitset<MAX_COMPONENTS> GenerateBitFlag(bool& isInitialized, int& ID, std::bitset<MAX_COMPONENTS>& signature);

private:
  //! Internal get next id for initializing component ID value
  static int NextID(bool& isInitialized, int& value);

};

//______________________________________________________________________________
inline std::bitset<MAX_COMPONENTS> ComponentIDGenerator::GenerateBitFlag(bool& isInitialized, int& ID, std::bitset<MAX_COMPONENTS>& signature)
{
  ID = NextID(isInitialized, ID);
  signature.set(ID);
  return signature;
}

//______________________________________________________________________________
inline int ComponentIDGenerator::NextID(bool& isInitialized, int& value)
{
  static int ID = 0;

  if (isInitialized)
    return value;

  isInitialized = true;
  value = ID++;
  return value;
}
