#include "Components/DebugComponent/DebugComponent.h"
#include "Systems/DebugSystem.h"
#include "DebugGUI/GUIController.h"

IDebugComponent::IDebugComponent(const char* groupName) : debugGroup(groupName) {}

IDebugComponent::IDebugComponent(const IDebugComponent& other)
{
  if (other.debugID >= 0)
    this->debugID = -1;
}

IDebugComponent::~IDebugComponent()
{
  if (debugID >= 0 && GUIController::IsAlive())
    GUIController::Get().RemoveImguiWindowFunction("Debug Components", debugID);
}

IDebugComponent& IDebugComponent::operator=(const IDebugComponent& other)
{
  this->debugID = -1;
  return *this;
}

IDebugComponent& IDebugComponent::operator=(IDebugComponent&& other) noexcept
{
  if (this->debugID < 0)
  {
    this->debugID = other.debugID;
    other.debugID = -1;
  }
  return *this;
}

void IDebugComponent::OnAdd(const EntityID& entity)
{
  DebugSystem::Register(*this, entity);
}

void IDebugComponent::OnRemove(const EntityID& entity)
{
  DebugSystem::Deregister(*this);
}
