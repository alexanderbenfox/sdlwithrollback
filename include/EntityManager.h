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

  EntityID RegisterEntity()
  {
    if (_livingEntityCount < MAX_ENTITIES)
    {
      EntityID id = _availableEntities.front();
      _availableEntities.pop();
      _livingEntityCount++;
      return id;
    }
    return MAX_ENTITIES;
  }

  void DestroyEntity(EntityID id)
  {
    if (id < MAX_ENTITIES)
    {
      // invalidate signature of destroyed entity
      _signatures[id].reset();

      // destroy id at the back of queue as it is newly available
      _availableEntities.push(id);
      _livingEntityCount--;
    }
  }

  //! Put this entity's signature into the array
  void SetSignature(EntityID id, ComponentBitFlag signature)
  {
    if (id < MAX_ENTITIES)
    {
      _signatures[id] = signature;
    }
  }

  //! Get this entity's signature from the array
  ComponentBitFlag GetSignature(EntityID id)
  {
    assert(id < MAX_ENTITIES && "Entity out of range.");
    return _signatures[id];
  }

  template <typename T = IComponent>
  void AddComponent()
  {

  }

  template <typename T = IComponent>
  void RemoveComponent()
  {

  }



private:
  //! initialize queue of available with all possible entity ids
  EntityManager() : _livingEntityCount(0)
  {

    for (EntityID entity = 0; entity < MAX_ENTITIES; entity++)
      _availableEntities.push(entity);
  }

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
