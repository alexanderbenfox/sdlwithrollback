#include "Core/ECS/ECSCoordinator.h"
#include "Core/ECS/Entity.h";

//______________________________________________________________________________
std::bitset<MAX_COMPONENTS> ECSCoordinator::RegisterComponent(int& id, ComponentEntityFnSet fnSet)
{
  // Generate component look up ID
  id = NextID();
  // Set runtime add, remove, serialize functions for this component
  _serializationHelpers[id] = fnSet;

  // return generated signature
  std::bitset<MAX_COMPONENTS> signature;
  signature.set(id);
  return signature;
}

//______________________________________________________________________________
std::function<void()> ECSCoordinator::AddSelf(EntityID entity, int componentID)
{
  return _serializationHelpers[componentID].AddSelf(entity);
}

//______________________________________________________________________________
void ECSCoordinator::RemoveSelf(EntityID entity, int componentID)
{
  _serializationHelpers[componentID].RemoveSelf(entity);
}

//______________________________________________________________________________
/*void ECSCoordinator::CopyComponentData(EntityID original, EntityID newEntity, int componentID)
{
  SBuffer buffer = _serializationHelpers[componentID].CopyDataToBuffer(original);
  _serializationHelpers[componentID].CopyFromBuffer(newEntity, buffer);
}*/

//______________________________________________________________________________
int ECSCoordinator::NextID()
{
  static int ID = 0;

  int rValue = ID++;

  // update our number of registered components on each generation
  ECSGlobalStatus::NRegisteredComponents = rValue;
  return rValue;
}
