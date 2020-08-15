#pragma once
#include <string>
#include "Core/ECS/IComponent.h"

struct IDebugComponent : public IComponent
{
  IDebugComponent() : IComponent() {}
  IDebugComponent(const char* groupName);
  ~IDebugComponent();

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