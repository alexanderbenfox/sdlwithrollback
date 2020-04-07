#pragma once

class Animation;

//! Component that indicates the current state of the entity
class IStateComponent : public IComponent
{
public:
  //!
  IStateComponent(std::shared_ptr<Entity> owner) : IComponent(owner) {}
  //!
  virtual ~IStateComponent() {}
  //!
  virtual int GetRemainingFrames() = 0;
};

class AttackStateComponent : public IStateComponent
{
public:
  //!
  AttackStateComponent(std::shared_ptr<Entity> owner) : lastFrame(-1), _lastAnimationFrame(-1), _attackAnim(nullptr), IStateComponent(owner) {}
  //!
  virtual ~AttackStateComponent() override
  {
    if (auto renderer = _owner->GetComponent<AnimationRenderer>())
    {
      //reset color back to white in case stuck in frame advantage
      renderer->SetDisplayColor(255, 255, 255);
    }
  }

  //!
  void SetAnimation(Animation* animation) { _attackAnim = animation; }

  //! Checks if an event should be trigger this frame of animation and calls its callback if so
  void CheckEvents(int frame, double x, double y, Transform* transform)
  {
    int fpsFrame = frame;
    //adjust to get the animation frame
    /*frame = (int)std::floorf((float)frame / gameFramePerAnimationFrame);
    if (frame == _lastAnimationFrame)
      return;
    _lastAnimationFrame = frame;*/

    for (int i = 0; i < _inProgressEvents.size(); i++)
    {
      AnimationEvent* evt = _inProgressEvents[i];
      if (frame >= evt->GetEndFrame())
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

  virtual int GetRemainingFrames() override
  {
    return (_attackAnim->GetFrameCount() - 1) - lastFrame;
  }

  int lastFrame = -1;
  
private:
  //! Map of frame starts for events to the event that should be triggered
  Animation* _attackAnim;
  std::vector<AnimationEvent*> _inProgressEvents;
  int _lastAnimationFrame = -1;

};

template <> struct ComponentTraits<AttackStateComponent>
{
  static const uint64_t GetSignature() { return 1 << 10; }
};

class HitStateComponent : public IStateComponent
{
public:
  //!
  HitStateComponent(std::shared_ptr<Entity> owner) : _linkedTimer(nullptr), IStateComponent(owner) {}
  //!
  virtual ~HitStateComponent() override
  {
    if (auto renderer = _owner->GetComponent<AnimationRenderer>())
    {
      //reset color back to white in case stuck in frame advantage
      renderer->SetDisplayColor(255, 255, 255);
    }
  }
  //!
  void SetTimer(TimerComponent* timer) { _linkedTimer = timer; }
  //!
  virtual int GetRemainingFrames() override { return (_linkedTimer->TotalFrames() - 1) - _linkedTimer->currFrame; }

private:
  //!
  TimerComponent* _linkedTimer;
};

template <> struct ComponentTraits<HitStateComponent>
{
  static const uint64_t GetSignature() { return 1 << 11; }
};