#include "DebugGUI/DebugItem.h"
#include "DebugGUI/GUIController.h"

DebugItem::DebugItem(const char* groupName) : debugGroup(groupName)
{
  std::function<void()> func = [this]()
  {
    ImGui::BeginGroup();
    OnDebug();
    ImGui::EndGroup();
  };

  debugID = GUIController::Get().AddImguiWindowFunction(debugGroup, func);
}

DebugItem& DebugItem::operator=(const DebugItem& other)
{
  this->debugID = 0;
  return *this;
}

DebugItem& DebugItem::operator=(DebugItem&& other)
{
  this->debugID = 0;
  return *this;
}

DebugItem::~DebugItem()
{
  if(debugID > 0)
    GUIController::Get().RemoveImguiWindowFunction(debugGroup, debugID);
}