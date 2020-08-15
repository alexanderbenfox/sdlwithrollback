#include "Core/ECS/EntityManager.h"

EntityID EntityManager::RegisterEntity()
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

void EntityManager::DestroyEntity(EntityID id)
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

void EntityManager::SetSignature(EntityID id, ComponentBitFlag signature)
{
  if (id < MAX_ENTITIES)
  {
    _signatures[id] = signature;
  }
}

ComponentBitFlag EntityManager::GetSignature(EntityID id)
{
  assert(id < MAX_ENTITIES && "Entity out of range.");
  return _signatures[id];
}

EntityManager::EntityManager() : _livingEntityCount(0)
{

  for (EntityID entity = 0; entity < MAX_ENTITIES; entity++)
    _availableEntities.push(entity);
}
