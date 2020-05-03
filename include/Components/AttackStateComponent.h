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
  AttackStateComponent(std::shared_ptr<Entity> owner) : lastFrame(-1), _attackAnim(nullptr), IStateComponent(owner) {}
  //!
  virtual ~AttackStateComponent() override
  {
    ClearEvents();
    if (auto properties = _owner->GetComponent<RenderProperties>())
    {
      //reset color back to white in case stuck in frame advantage
      properties->SetDisplayColor(255, 255, 255);
    }
  }

  //!
  void SetAnimation(Animation* animation) { _attackAnim = animation; }

  //!
  void ClearEvents()
  {
    for(auto& event : inProgressEvents)
      event->EndEvent();
    inProgressEvents.clear();
  }

  AnimationEvent* GetEventStartsThisFrame(int frame)
  {
    auto evtIter = _attackAnim->Events().find(frame);
    if (evtIter != _attackAnim->Events().end())
    {
      return &evtIter->second;
    }
    return nullptr;
  }

  virtual int GetRemainingFrames() override
  {
    return (_attackAnim->GetFrameCount() - 1) - lastFrame;
  }

  int lastFrame = -1;
  std::vector<AnimationEvent*> inProgressEvents;
  
private:
  //! Map of frame starts for events to the event that should be triggered
  Animation* _attackAnim;

};

class HitStateComponent : public IStateComponent
{
public:
  //!
  HitStateComponent(std::shared_ptr<Entity> owner) : _linkedTimer(nullptr), IStateComponent(owner) {}
  //!
  virtual ~HitStateComponent() override
  {
    if (auto properties = _owner->GetComponent<RenderProperties>())
    {
      //reset color back to white in case stuck in frame advantage
      properties->SetDisplayColor(255, 255, 255);
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
