#pragma once
#include "Components/Transform.h"
#include "Components/StateComponent.h"

#include <functional>

//______________________________________________________________________________
class AnimationEvent
{
public:
  enum class Type
  {
    Hitbox, Throwbox, Movement, EntitySpawner
  };

  AnimationEvent(int startFrame, int duration, std::function<void(EntityID, Transform*, StateComponent*)> onTriggerCallback,
    std::vector<std::function<void(EntityID, Transform*, StateComponent*)>> update, std::function<void(EntityID)> onEndCallback, Type type) :
    _frame(startFrame), _duration(duration), _onTrigger(onTriggerCallback), _updates(update), _onEnd(onEndCallback), type(type) {}

  void TriggerEvent(EntityID id, Transform* trans, StateComponent* state) { _onTrigger(id, trans, state); }
  void UpdateEvent(int frame, EntityID id, Transform* trans, StateComponent* state) { _updates[frame - _frame - 1](id, trans, state); }
  void EndEvent(EntityID id) { _onEnd(id); }

  int GetStartFrame() { return _frame; }
  int GetEndFrame() { return _frame + _duration; }

  static void EndHitboxEvent(EntityID entity);
  static void EndThrowboxEvent(EntityID entity);
  static void EndMovementEvent(EntityID entity);
  static void EndEntitySpawnEvent(EntityID entity);

  Type type;

private:
  //! Frame this event will be called on
  int _frame = 0;
  int _duration = 0;
  //!
  std::function<void(EntityID, Transform*, StateComponent*)> _onTrigger;
  std::vector<std::function<void(EntityID, Transform*, StateComponent*)>> _updates;
  std::function<void(EntityID)> _onEnd;
};

//! Data structure that links a frame of animation to an event that starts on that frame
typedef std::vector<std::vector<AnimationEvent>> EventList;

struct EventBuilderDictionary
{
  // translates the frames on sprite sheet to frames we want to play in order
  std::vector<int> realFrameToSheetFrame;
  std::vector<std::vector<int>> sheetFrameToRealFrame;
};

struct AnimationEventHelper
{
  static EventList BuildEventList(const Vector2<int> offset, const std::vector<AnimationActionEventData>& animEventData, const FrameData& frameData, int totalSheetFrames, std::vector<int>& animFrameToSheetFrame);
  //! Translates the animation in sprite sheet to variable frame data values
  static EventBuilderDictionary ParseAnimationEventList(const std::vector<AnimationActionEventData>& animEventData, const FrameData& frameData, int totalSheetFrames);
};
