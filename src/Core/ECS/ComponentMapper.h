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
  std::function<void(EntityID, std::ostream&)> SerializeSelf;
  std::function<void(EntityID, std::istream&)> DeserializeSelf;
  //std::function<SBuffer(EntityID)> CopyDataToBuffer;
  //std::function<void(EntityID, const SBuffer&)> CopyFromBuffer;
  std::type_index typeIndex = typeid(int);

};

//______________________________________________________________________________
//! Maps components to add/remove and serialization functions at runtime (ID map is based on order of initialization of components)
class ComponentMapper
{
public:
  //! Static getter
  static ComponentMapper& Get()
  {
    static ComponentMapper manager;
    return manager;
  }

  //! Generate bit flag for component (used on ComponentTraits initialization)
  std::bitset<MAX_COMPONENTS> GenerateBitFlag(int& id, std::bitset<MAX_COMPONENTS>& signature, ComponentEntityFnSet fnSet);
  //! Adds component mapped to ID to the entity and returns a removal function for it
  std::function<void()> AddSelf(EntityID entity, int componentID);
  //! Removes component mapped to ID from the entity
  void RemoveSelf(EntityID entity, int componentID);
  //! Gets the type index of the component ID so that entity can look it up via deleter functions
  std::type_index GetTypeIndex(int componentID) { return _serializationHelpers[componentID].typeIndex; }
  //! Serializes component data to stream if entity has component and if it is serializable
  void SerializeComponent(EntityID entity, std::ostream& os, int componentID) { _serializationHelpers[componentID].SerializeSelf(entity, os); }
  //! Deserializes component data from stream if entity has comp and if it is serializable
  void DeserializeComponent(EntityID entity, std::istream& is, int componentID) { _serializationHelpers[componentID].DeserializeSelf(entity, is); }

  //!
//void CopyComponentData(EntityID original, EntityID newEntity, int componentID);

private:
  //! Internal get next id for initializing component ID value
  int NextID();
  //! Map ID value (which will correspond to its location in the bit map)
  std::array<ComponentEntityFnSet, MAX_COMPONENTS> _serializationHelpers;

};
