#include "Core/ECS/ComponentIDGenerator.h"
#include "Core/ECS/Entity.h";

//______________________________________________________________________________
std::array<ComponentEntityFnSet, MAX_COMPONENTS> ComponentIDGenerator::_serializationHelpers;

//______________________________________________________________________________
std::bitset<MAX_COMPONENTS> ComponentIDGenerator::GenerateBitFlag(bool& isInitialized, int& ID, std::bitset<MAX_COMPONENTS>& signature, ComponentEntityFnSet fnSet)
{
  bool addFnSets = !isInitialized;
  ID = NextID(isInitialized, ID);

  if (addFnSets)
  {
    _serializationHelpers[ID] = fnSet;
  }

  signature.set(ID);
  return signature;
}

//______________________________________________________________________________
std::function<void()> ComponentIDGenerator::AddSelf(EntityID entity, int componentID)
{
  return _serializationHelpers[componentID].AddSelf(entity);
}

//______________________________________________________________________________
void ComponentIDGenerator::RemoveSelf(EntityID entity, int componentID)
{
  _serializationHelpers[componentID].RemoveSelf(entity);
}

//______________________________________________________________________________
void ComponentIDGenerator::CopyComponentData(EntityID original, EntityID newEntity, int componentID)
{
  SBuffer buffer = _serializationHelpers[componentID].SerializeSelf(original);
  _serializationHelpers[componentID].CopyFromBuffer(newEntity, buffer);
}

//______________________________________________________________________________
int ComponentIDGenerator::NextID(bool& isInitialized, int& value)
{
  static int ID = 0;

  if (isInitialized)
    return value;

  isInitialized = true;
  value = ID++;

  // update our number of registered components on each generation
  ECSGlobalStatus::NRegisteredComponents = ID;
  return value;
}
