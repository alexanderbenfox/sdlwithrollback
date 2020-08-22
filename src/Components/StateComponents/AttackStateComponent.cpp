#include "Components/StateComponents/AttackStateComponent.h"
#include "Components/RenderComponent.h"

AttackStateComponent::AttackStateComponent() :
  lastFrame(-1), IComponent()
{
}

void AttackStateComponent::OnRemove(const EntityID& entity)
{
  ClearEvents(entity);
  //reset color back to white in case stuck in frame advantage
  if (ComponentArray<RenderProperties>::Get().HasComponent(entity))
    ComponentArray<RenderProperties>::Get().GetComponent(entity).SetDisplayColor(255, 255, 255);
}

void AttackStateComponent::ClearEvents(const EntityID& entity)
{
  /*for(auto& event : inProgressEvents)
    event->EndEvent(entity);
  inProgressEvents.clear();*/

  for (const AnimationEvent::Type& type: inProgressEventTypes)
  {
    if (type == AnimationEvent::Type::EntitySpawner)
      AnimationEvent::EndEntitySpawnEvent(entity);
    else if (type == AnimationEvent::Type::Hitbox)
      AnimationEvent::EndHitboxEvent(entity);
    else if (type == AnimationEvent::Type::Throwbox)
      AnimationEvent::EndThrowboxEvent(entity);
    else if (type == AnimationEvent::Type::Movement)
      AnimationEvent::EndMovementEvent(entity);
  }
  inProgressEventTypes.clear();
}

void AttackStateComponent::Serialize(std::ostream& os) const
{
  Serializer<std::string>::Serialize(os, attackAnimation);
  Serializer<int>::Serialize(os, lastFrame);

  //serialize event list
  //write in n entries in set
  int types = (int)inProgressEventTypes.size();
  Serializer<int>::Serialize(os, types);
  for (const AnimationEvent::Type& type : inProgressEventTypes)
  {
    Serializer<AnimationEvent::Type>::Serialize(os, type);
  }
}

void AttackStateComponent::Deserialize(std::istream& is)
{
  Serializer<std::string>::Deserialize(is, attackAnimation);
  Serializer<int>::Deserialize(is, lastFrame);

  int nTypes = 0;
  Serializer<int>::Deserialize(is, nTypes);
  for (int i = 0; i < nTypes; i++)
  {
    AnimationEvent::Type type;
    Serializer<AnimationEvent::Type>::Deserialize(is, type);
    inProgressEventTypes.insert(type);
  }
}
