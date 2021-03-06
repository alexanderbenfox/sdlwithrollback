#include "Components/DebugComponent/DebugComponent.h"
#include "DebugGUI/GUIController.h"

IDebugComponent::IDebugComponent(const char* groupName) : debugGroup(groupName) {}

IDebugComponent::IDebugComponent(const IDebugComponent& other)
{
  if (other.debugID >= 0)
    this->debugID = -1;
}

IDebugComponent::~IDebugComponent()
{
  if (debugID >= 0)
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
  entityID = entity;
  if (debugID < 0)
  {
    std::function<void()> func = [this]() { OnDebug(); };
    debugID = GUIController::Get().AddImguiWindowFunction("Debug Components", debugGroup, func);
  }
}

void IDebugComponent::OnRemove(const EntityID& entity)
{
  if (debugID >= 0)
    GUIController::Get().RemoveImguiWindowFunction("Debug Components", debugID);
  debugID = -1;
}
