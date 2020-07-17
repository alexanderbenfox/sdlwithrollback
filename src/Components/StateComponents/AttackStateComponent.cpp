#include "Components/StateComponents/AttackStateComponent.h"
#include "Components/RenderComponent.h"

AttackStateComponent::AttackStateComponent(std::shared_ptr<Entity> owner) :
  lastFrame(-1), _attackAnim(nullptr), IComponent(owner)
{
}

AttackStateComponent::~AttackStateComponent()
{
  ClearEvents();
  //reset color back to white in case stuck in frame advantage
  if (auto properties = _owner->GetComponent<RenderProperties>())
    properties->SetDisplayColor(255, 255, 255);
}

void AttackStateComponent::Init(Animation* animation, std::shared_ptr<EventList> eventList)
{
  _attackAnim = animation;
  _eventList = eventList;
}

void AttackStateComponent::ClearEvents()
{
  for(auto& event : inProgressEvents)
    event->EndEvent(_owner->GetComponent<Transform>().get());
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
