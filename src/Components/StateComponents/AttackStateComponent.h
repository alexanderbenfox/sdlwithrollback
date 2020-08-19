#pragma once
#include "Core/ECS/IComponent.h"
#include "AssetManagement/Animation.h"

class AttackStateComponent : public IComponent
{
public:
  //! Constructor
  AttackStateComponent();
  //! Clears events and resets debug color
  void OnRemove(const EntityID& entity) override;
  //! Clear event list
  void ClearEvents(const EntityID& entity);
  //! List of in progress events
  std::vector<AnimationEvent*> inProgressEvents;

  //!
  std::string attackAnimation;
  //! Last frame visited
  int lastFrame = -1;

};
