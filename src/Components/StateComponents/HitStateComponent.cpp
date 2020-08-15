#include "Components/StateComponents/HitStateComponent.h"
#include "Components/StateComponent.h"
#include "Components/RenderComponent.h"
#include "Core/ECS/Entity.h"

HitStateComponent::HitStateComponent() :
_linkedTimer(nullptr), IComponent()
{
}

void HitStateComponent::OnRemove(const EntityID& entity)
{
  if (ComponentArray<RenderProperties>::Get().HasComponent(entity))
  {
    //reset color back to white in case stuck in frame advantage
    ComponentArray<RenderProperties>::Get().GetComponent(entity).SetDisplayColor(255, 255, 255);
  }
}

void HitStateComponent::SetTimer(TimedActionComponent* timer)
{
  _linkedTimer = timer;
}

int HitStateComponent::GetRemainingFrames()
{
  return _linkedTimer->totalFrames - _linkedTimer->currFrame;
}
