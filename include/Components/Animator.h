#pragma once
#include "Components/Sprite.h"
#include "ListenerInterfaces.h"

const float gameFramePerAnimationFrame = 60.0f / animation_fps;

class Animation;

class AnimationEvent
{
public:
  AnimationEvent(int startFrame, int duration, std::function<void(double, double, Transform*)> onTriggerCallback, std::vector<std::function<void(double,double,Transform*)>> update, std::function<void()> onEndCallback) :
    _frame(startFrame), _duration(duration), _onTrigger(onTriggerCallback), _updates(update), _onEnd(onEndCallback) {}

  void TriggerEvent(double x, double y, Transform* trans) { _onTrigger(x, y, trans); }
  void UpdateEvent(int frame, double x, double y, Transform* trans) { _updates[frame - _frame - 1](x, y, trans); }
  void EndEvent() { _onEnd(); }
  int GetEndFrame() { return _frame + _duration; }

private:
  //! Delete copy constructor because it will only be used in the animation
  AnimationEvent(const AnimationEvent&) = delete;
  AnimationEvent operator=(AnimationEvent&) = delete;
  //! Frame this event will be called on
  int _frame;
  int _duration;
  //!
  std::function<void(double, double, Transform*)> _onTrigger;
  std::vector<std::function<void(double,double,Transform*)>> _updates;
  std::function<void()> _onEnd;
};

class Animation
{
public:
  Animation(const char* sheet, int rows, int columns, int startIndexOnSheet, int frames);

  void AddHitboxEvents(const char* hitboxesSheet, FrameData frameData, std::shared_ptr<Entity> entity);

  //! Translates anim frame to the frame on spritesheet
  SDL_Rect GetFrameSrcRect(int animFrame);

  const int GetFrameCount() { return static_cast<int>(_animFrameToSheetFrame.size()); }

  Vector2<int> const GetFrameWH() { return _frameSize; }

  Vector2<int> const GetRefPxLocation() { return _referencePx; }
  //! Checks if an event should be trigger this frame of animation and calls its callback if so
  std::unordered_map<int, AnimationEvent>& Events() { return _events; }

protected:
  //! Gets first non-transparent pixel from the top left
  Vector2<int> FindReferencePixel(const char* sheet);
  //!
  int _rows, _columns, _frames, _startIdx;
  //!
  Vector2<int> _frameSize;
  //!
  Vector2<int> _referencePx;
  //! Map of frame starts for events to the event that should be triggered
  std::unordered_map<int, AnimationEvent> _events;
  //!
  std::vector<int> _animFrameToSheetFrame;

};

class Animator : public IComponent
{
public:
  Animator(std::shared_ptr<Entity> owner);

  void RegisterAnimation(const std::string& name, const char* sheet, int rows, int columns, int startIndexOnSheet, int frames);

  // Setter function
  void Play(const std::string& name, bool isLooped, bool horizontalFlip);

  void ChangeListener(IAnimatorListener* listener) { _listener = listener; }

  Animation* GetAnimationByName(const std::string& name)
  {
    if(_animations.find(name) != _animations.end())
      return &_animations.find(name)->second;
    return nullptr;
  }

  //!
  const std::function<int(int)>& GetNextFrame() const { return _nextFrameOp; }
  //!
  IAnimatorListener* GetListener() { return _listener; }
  //!
  Animation& GetCurrentAnimation() { return _currentAnimation->second; }

  // STATE VARIABLES
  //!
  bool playing;
  //!
  bool looping;
  //!
  float accumulatedTime;
  //!
  int frame;
  

  friend std::ostream& operator<<(std::ostream& os, const Animator& animator);
  friend std::istream& operator>>(std::istream& is, Animator& animator);

protected:
  //! Things that need to know when an animation is done
  IAnimatorListener* _listener;
  //! All animations registered to this animator
  std::unordered_map<std::string, Animation> _animations;
  //!
  std::unordered_map<std::string, Animation>::iterator _currentAnimation;
  //!
  std::string _currentAnimationName;
  
  //!
  Vector2<int> _basisOffset;
  //!
  Vector2<int> _basisRefPx;

  std::vector<std::function<void(Animation*)>> _onAnimCompleteCallbacks;

};

template <> struct ComponentTraits<AnimationRenderer>
{
  static const uint64_t GetSignature() { return 1 << 6;}
};