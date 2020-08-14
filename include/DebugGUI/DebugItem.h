#pragma once
#include <string>
#include "DebugGUI/GUIController.h"

/*struct DebugItem
{
  //! Empty constructor won't make the item added to DebugGUI
  DebugItem() = default;
  //! Adds on debug function to the debug gui
  DebugItem(const char* groupName);
  //! if we are copying this object, we do not want it to create another debug function
  DebugItem(const DebugItem& other);
  virtual DebugItem& operator=(const DebugItem& other);
  virtual DebugItem& operator=(DebugItem&& other) noexcept;
  //! destroy the function from the gui controller
  virtual ~DebugItem();

  int debugID = 0;
  const std::string debugGroup;
  virtual void OnDebug() = 0;
};*/

#include "Components/IComponent.h"

struct IDebugComponent : public IComponent
{
  IDebugComponent() : IComponent() {}
  IDebugComponent(const char* groupName);
  //! if we are copying this object, we do not want it to create another debug function
  IDebugComponent(const IDebugComponent& other);
  virtual IDebugComponent& operator=(const IDebugComponent& other);
  virtual IDebugComponent& operator=(IDebugComponent&& other) noexcept;

  virtual void OnAdd(const EntityID& entity) override;
  virtual void OnRemove(const EntityID& entity) override;
  virtual void OnDebug() = 0;

  EntityID entityID = -1;
  int debugID = -1;
  const std::string debugGroup;
};