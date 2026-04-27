#pragma once
#include "Components/Transform.h"
#include "Components/StateComponent.h"
#include "AssetManagement/EditableAssets/ActionAsset.h"

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

//! Game-frame-indexed event data in game space (format-agnostic)
struct GameFrameEvent
{
  Rect<double> hitbox;         // in game space (already scaled)
  Vector2<float> movement;
  ProjectileData create;
  bool isActive = false;
};

//! Complete action timeline ready for BuildEventList (format-agnostic)
struct ActionTimeline
{
  std::vector<GameFrameEvent> frames;  // one entry per game frame
  FrameData frameData;
  Vector2<float> hitboxOffset;         // offset from transform center (in game space)
  std::vector<int> visualFrameMap;     // game frame → source frame (for animation sync)
};

struct AnimationEventHelper
{
  //! Build runtime EventList from a format-agnostic ActionTimeline
  static EventList BuildEventList(const ActionTimeline& timeline);

  //! Convert sprite-space EventData into a game-space ActionTimeline
  static ActionTimeline ResolveSpriteTimeline(
      const std::vector<EventData>& spriteFrameEvents,
      const FrameData& frameData,
      int totalSheetFrames,
      const Vector2<float>& textureScalingFactor,
      AnchorPoint anchorPt,
      const Vector2<float>& scaledAnchorOffset);

private:
  //! Translates the animation in sprite sheet to variable frame data values
  static EventBuilderDictionary ParseAnimationEventList(const std::vector<EventData>& animEventData, const FrameData& frameData, int totalSheetFrames);
};
