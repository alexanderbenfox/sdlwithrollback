#pragma once
#include <queue>
#include <array>
#include <bitset>
#include <cassert>

#include "Globals.h"

//! Bit flag for the components currently attached
using ComponentBitFlag = std::bitset<MAX_COMPONENTS>;

class EntityManager
{
public:
  //! Get Singleton instance of the EntityManager
  static EntityManager& Get()
  {
    static EntityManager manager;
    return manager;
  }
  //! Get new ID from available IDs
  EntityID RegisterEntity();
  //! Reset the signatures on entity and free the ID
  void DestroyEntity(EntityID id);
  //! Put this entity's signature into the array
  void SetSignature(EntityID id, ComponentBitFlag signature);
  //! Get this entity's signature from the array
  ComponentBitFlag GetSignature(EntityID id);

private:
  //! initialize queue of available with all possible entity ids
  EntityManager();

  //! Delete any sort of copy for singleton
  EntityManager(const EntityManager&) = delete;
  EntityManager(EntityManager&&) = delete;
  EntityManager operator=(const EntityManager&) = delete;
  EntityManager operator=(EntityManager&&) = delete;

  //! queue of unused entity ids
  std::queue<EntityID> _availableEntities;
  //! array of signatures where index corresponds to entity id
  std::array<ComponentBitFlag, MAX_ENTITIES> _signatures;
  //! total living entities - used to keep limits on how many exist
  uint32_t _livingEntityCount;

};
