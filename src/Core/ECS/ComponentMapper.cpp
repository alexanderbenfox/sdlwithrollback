#include "Core/ECS/ComponentMapper.h"
#include "Core/ECS/Entity.h";

//______________________________________________________________________________
std::bitset<MAX_COMPONENTS> ComponentMapper::GenerateBitFlag(int& id, std::bitset<MAX_COMPONENTS>& signature, ComponentEntityFnSet fnSet)
{
  id = NextID();
  _serializationHelpers[id] = fnSet;

  signature.set(id);
  return signature;
}

//______________________________________________________________________________
std::function<void()> ComponentMapper::AddSelf(EntityID entity, int componentID)
{
  return _serializationHelpers[componentID].AddSelf(entity);
}

//______________________________________________________________________________
void ComponentMapper::RemoveSelf(EntityID entity, int componentID)
{
  _serializationHelpers[componentID].RemoveSelf(entity);
}

//______________________________________________________________________________
/*void ComponentMapper::CopyComponentData(EntityID original, EntityID newEntity, int componentID)
{
  SBuffer buffer = _serializationHelpers[componentID].CopyDataToBuffer(original);
  _serializationHelpers[componentID].CopyFromBuffer(newEntity, buffer);
}*/

//______________________________________________________________________________
int ComponentMapper::NextID()
{
  static int ID = 0;

  int rValue = ID++;

  // update our number of registered components on each generation
  ECSGlobalStatus::NRegisteredComponents = rValue;
  return rValue;
}
