#include "Systems/DebugSystem.h"
#include "Components/DebugComponent/DebugComponent.h"
#include "DebugGUI/GUIController.h"

//______________________________________________________________________________
void DebugSystem::Register(IDebugComponent& comp, EntityID entity)
{
  comp.entityID = entity;
  if (comp.debugID < 0)
  {
    std::function<void()> func = [&comp]() { comp.OnDebug(); };
    comp.debugID = GUIController::Get().AddImguiWindowFunction("Debug Components", comp.debugGroup, func);
  }
}

//______________________________________________________________________________
void DebugSystem::Deregister(IDebugComponent& comp)
{
  if (comp.debugID >= 0)
    GUIController::Get().RemoveImguiWindowFunction("Debug Components", comp.debugID);
  comp.debugID = -1;
}
