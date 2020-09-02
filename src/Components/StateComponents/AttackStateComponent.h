#pragma once
#include "Core/ECS/IComponent.h"
#include "AssetManagement/Animation.h"

#include <unordered_set>

class AttackStateComponent : public IComponent, public ISerializable
{
public:
  //! Constructor
  AttackStateComponent();
  //! Clears events and resets debug color
  void OnRemove(const EntityID& entity) override;
  //! Clear event list
  void ClearEvents(const EntityID& entity);
  //! List of in progress events
  //std::vector<AnimationEvent*> inProgressEvents;
  std::unordered_set<AnimationEvent::Type> inProgressEventTypes;
  //!
  std::string attackAnimation;
  //! Last frame visited
  int lastFrame = -1;

  void Serialize(std::ostream& os) const override;
  void Deserialize(std::istream& is) override;
  std::string Log() override;

};
