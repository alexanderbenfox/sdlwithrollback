#include "Components/StateComponents/HitStateComponent.h"
#include "Components/StateComponent.h"
#include "Components/RenderComponent.h"
#include "Entity.h"

HitStateComponent::HitStateComponent(std::shared_ptr<Entity> owner) :
_linkedTimer(nullptr), IComponent(owner)
{
}

HitStateComponent::~HitStateComponent()
{
  if (auto properties = _owner->GetComponent<RenderProperties>())
  {
    //reset color back to white in case stuck in frame advantage
    properties->SetDisplayColor(255, 255, 255);
  }
}

void HitStateComponent::SetTimer(ActionTimer* timer)
{
  _linkedTimer = timer;
}

int HitStateComponent::GetRemainingFrames()
{
  return _linkedTimer->Duration() - _linkedTimer->currFrame;
}
