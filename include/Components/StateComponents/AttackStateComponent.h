#pragma once
#include "Components/IComponent.h"
#include "AssetManagement/Animation.h"

class AttackStateComponent : public IComponent
{
public:
  //! Constructor
  AttackStateComponent(std::shared_ptr<Entity> owner);
  //! Clears events and resets debug color
  virtual ~AttackStateComponent() override;
  //! Sets animation and event list pointers
  void Init(Animation* animation, std::shared_ptr<EventList> eventList);
  //! Clear event list
  void ClearEvents();
  //! If an event is starting on this frame, returns it. Otherwise error
  std::vector<AnimationEvent>& GetEventsStarting(int frame);
  //! Gets remaining frames in attack animation
  int GetRemainingFrames();
  //! Last frame visited
  int lastFrame = -1;
  //! List of in progress events
  std::vector<AnimationEvent*> inProgressEvents;
  
private:
  //! Map of frame starts for events to the event that should be triggered
  Animation* _attackAnim;
  // List of events for each frame of animation
  std::shared_ptr<EventList> _eventList;

};
