#pragma once

class Animation;

class AttackStateComponent : public IComponent
{
public:
  //!
  AttackStateComponent(std::shared_ptr<Entity> owner) : lastFrame(-1), IComponent(owner) {}

  //!
  void SetAnimation(Animation* animation) { _attackAnim = animation; }

  //! Checks if an event should be trigger this frame of animation and calls its callback if so
  void CheckEvents(int frame, double x, double y, Transform* transform)
  {
    for (int i = 0; i < _inProgressEvents.size(); i++)
    {
      AnimationEvent* evt = _inProgressEvents[i];
      if (frame == evt->GetEndFrame())
      {
        evt->EndEvent();
        _inProgressEvents.erase(_inProgressEvents.begin() + i);
        i--;
      }
      else
      {
        evt->UpdateEvent(frame, x, y, transform);
      }
    }

    auto evtIter = _attackAnim->Events().find(frame);
    if (evtIter != _attackAnim->Events().end())
    {
      evtIter->second.TriggerEvent(x, y, transform);
      _inProgressEvents.push_back(&evtIter->second);
    }
  }
  //!
  void ClearEvents()
  {
    for(auto& event : _inProgressEvents)
      event->EndEvent();
    _inProgressEvents.clear();
  }

  int lastFrame = -1;
  
private:
  //! Map of frame starts for events to the event that should be triggered
  Animation* _attackAnim;
  std::vector<AnimationEvent*> _inProgressEvents;

};

template <> struct ComponentTraits<AttackStateComponent>
{
  static const uint64_t GetSignature() { return 1 << 10; }
};