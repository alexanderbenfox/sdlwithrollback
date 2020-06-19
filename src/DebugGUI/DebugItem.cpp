#include "DebugGUI/DebugItem.h"

DebugItem::DebugItem(const char* groupName) : debugGroup(groupName)
{
  std::function<void()> func = [this]() { OnDebug(); };
  debugID = GUIController::Get().AddImguiWindowFunction(debugGroup, func);
}

DebugItem& DebugItem::operator=(const DebugItem& other)
{
  this->debugID = -1;
  return *this;
}

DebugItem& DebugItem::operator=(DebugItem&& other) noexcept
{
  this->debugID = -1;
  return *this;
}

DebugItem::~DebugItem()
{
  if(debugID >= 0)
    GUIController::Get().RemoveImguiWindowFunction(debugGroup, debugID);
}
