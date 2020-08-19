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
  for(auto& event : inProgressEvents)
    event->EndEvent(entity);
  inProgressEvents.clear();
}
