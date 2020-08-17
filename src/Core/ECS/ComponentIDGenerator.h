#pragma once
#include "Globals.h"
#include "Core/Interfaces/Serializable.h"

#include <bitset>
#include <functional>
#include <typeindex>
#include <array>

class Entity;

struct ComponentEntityFnSet
{
  std::function<std::function<void()>(EntityID)> AddSelf;
  std::function<void(EntityID)> RemoveSelf;
  std::function<SBuffer(EntityID)> SerializeSelf;
  std::function<void(EntityID, const SBuffer&)> CopyFromBuffer;
  std::type_index typeIndex = typeid(int);

};

//______________________________________________________________________________
class ComponentIDGenerator
{
public:
  //! Generate bit flag for component (used on ComponentTraits initialization)
  static std::bitset<MAX_COMPONENTS> GenerateBitFlag(bool& isInitialized, int& ID, std::bitset<MAX_COMPONENTS>& signature, ComponentEntityFnSet fnSet);

  static std::function<void()> AddSelf(EntityID entity, int componentID);
  static void RemoveSelf(EntityID entity, int componentID);
  static void CopyComponentData(EntityID original, EntityID newEntity, int componentID);
  static std::type_index GetTypeIndex(int componentID) { return _serializationHelpers[componentID].typeIndex; }

private:
  //! Internal get next id for initializing component ID value
  static int NextID(bool& isInitialized, int& value);

  //! Map ID value (which will correspond to its location in the bit map)
  static std::array<ComponentEntityFnSet, MAX_COMPONENTS> _serializationHelpers;

};
