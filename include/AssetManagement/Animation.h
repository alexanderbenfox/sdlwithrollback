#pragma once
#include "Components/Transform.h"
#include "Components/StateComponent.h"
#include "AssetManagement/BlitOperation.h"
#include "ComponentConst.h"

#include <functional>
#include <unordered_map>

//!
const float secPerFrame = 1.0f / 60.0f;
const float gameFramePerAnimationFrame = (1.0f / secPerFrame) / animation_fps;

enum class AnchorPoint
{
  TL, TR, BL, BR, Size
};

class Animation;

//______________________________________________________________________________
class AnimationEvent
{
public:
  AnimationEvent(int startFrame, int duration, std::function<void(Transform*, StateComponent*)> onTriggerCallback, std::vector<std::function<void(Transform*, StateComponent*)>> update, std::function<void(Transform*)> onEndCallback) :
    _frame(startFrame), _duration(duration), _onTrigger(onTriggerCallback), _updates(update), _onEnd(onEndCallback) {}

  void TriggerEvent(Transform* trans, StateComponent* state) { _onTrigger(trans, state); }
  void UpdateEvent(int frame, Transform* trans, StateComponent* state) { _updates[frame - _frame - 1](trans, state); }
  void EndEvent(Transform* trans) { _onEnd(trans); }
  int GetEndFrame() { return _frame + _duration; }

private:
  //! Delete copy constructor because it will only be used in the animation
  AnimationEvent(const AnimationEvent&) = delete;
  AnimationEvent operator=(AnimationEvent&) = delete;
  //! Frame this event will be called on
  int _frame;
  int _duration;
  //!
  std::function<void(Transform*, StateComponent*)> _onTrigger;
  std::vector<std::function<void(Transform*, StateComponent*)>> _updates;
  std::function<void(Transform*)> _onEnd;
};

typedef std::tuple<int, int, std::function<void(Transform*, StateComponent*)>, std::vector<std::function<void(Transform*, StateComponent*)>>, std::function<void(Transform*)>> EventInitParams;

//______________________________________________________________________________
class Animation
{
public:
  Animation(const char* sheet, int rows, int columns, int startIndexOnSheet, int frames, AnchorPoint anchor);

  EventInitParams CreateHitboxEvent(const char* hitboxesSheet, FrameData frameData);

  //! Translates anim frame to the frame on spritesheet
  SDL_Rect GetFrameSrcRect(int animFrame) const;

  const int GetFrameCount() const { return static_cast<int>(_animFrameToSheetFrame.size()); }

  Vector2<int> GetFrameWH() const { return _frameSize; }
  //!
  Texture& GetSheetTexture() const;
  //!
  Resource<GLTexture>& GLGetSheetTexture() const;
  //! Gets first non-transparent pixel from the top left and bottom left
  Vector2<int> FindAnchorPoint(AnchorPoint anchorType, bool fromFirstFrame) const;
  //!
  std::pair<AnchorPoint, Vector2<int>> const& GetMainAnchor() const { return _anchorPoint; }

  int GetFlipMargin() const { return _frameSize.x - _lMargin; }

protected:
  //!
  int _rows, _columns, _frames, _startIdx;
  //!
  Vector2<int> _frameSize;
  //! stores the bottom left and top left reference pixels
  //Vector2<int> _anchorPoints[(const int)AnchorPoint::Size];
  //!
  std::vector<int> _animFrameToSheetFrame;
  //!
  std::string _src;
  //!
  std::pair<AnchorPoint, Vector2<int>> _anchorPoint;
  //! finding margin from the bottom right now
  int _lMargin, _rMargin, _tMargin;

};

typedef std::unordered_map<int, AnimationEvent> EventList;

//______________________________________________________________________________
class AnimationCollection
{
public:
  AnimationCollection() = default;
  void RegisterAnimation(const std::string& animationName, const char* sheet, int rows, int columns, int startIndexOnSheet, int frames, AnchorPoint anchor);
  void AddHitboxEvents(const std::string& animationName, const char* hitboxesSheet, FrameData frameData);


  Vector2<int> GetRenderOffset(const std::string& animationName, bool flipped, int transformWidth) const;
  //! Getters
  Animation* GetAnimation(const std::string& name)
  {
    if(_animations.find(name) == _animations.end())
      return nullptr;
    return &_animations.find(name)->second;
  }
  //!
  EventList* GetEventList(const std::string& name)
  {
    if(_events.find(name) == _events.end())
      return nullptr;
    return _events.find(name)->second.get();
  }

  std::unordered_map<std::string, Animation>::iterator GetAnimationIt(const std::string& name)
  {
    return _animations.find(name);
  }

  std::unordered_map<std::string, Animation>::iterator GetEnd()
  {
    return _animations.end();
  }

  bool IsValid(const std::unordered_map<std::string, Animation>::iterator& it)
  {
    return it != _animations.end();
  }

private:
  //! Map of animations name to animation object
  std::unordered_map<std::string, Animation> _animations;
  //! Map of frame starts for events to the event that should be triggered
  std::unordered_map<std::string, std::shared_ptr<EventList>> _events;
  //!
  Vector2<int> _anchorPoint[(const int)AnchorPoint::Size];
  Rect<int> _anchorRect;

  //! use the first sprite sheet in as anchor point reference
  bool _useFirstSprite = false;

};
