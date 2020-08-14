#include "Components/StateComponents/AttackStateComponent.h"
#include "Components/RenderComponent.h"

AttackStateComponent::AttackStateComponent() :
  lastFrame(-1), _attackAnim(nullptr), IComponent()
{
}

void AttackStateComponent::OnRemove(const EntityID& entity)
{
  ClearEvents(entity);
  //reset color back to white in case stuck in frame advantage
  if (ComponentArray<RenderProperties>::Get().HasComponent(entity))
    ComponentArray<RenderProperties>::Get().GetComponent(entity).SetDisplayColor(255, 255, 255);
}

void AttackStateComponent::Init(Animation* animation, std::shared_ptr<EventList> eventList)
{
  _attackAnim = animation;
  _eventList = eventList;
}

void AttackStateComponent::ClearEvents(const EntityID& entity)
{
  for(auto& event : inProgressEvents)
    event->EndEvent(entity);
  inProgressEvents.clear();
}

std::vector<AnimationEvent>& AttackStateComponent::GetEventsStarting(int frame)
{
  return (*_eventList)[frame];
}

int AttackStateComponent::GetRemainingFrames()
{
  return (_attackAnim->GetFrameCount() - 1) - lastFrame;
}
