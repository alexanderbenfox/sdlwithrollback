#pragma once
#include "Components/Sprite.h"
#include "ListenerInterfaces.h"

class Animation;

class AnimationEvent
{
public:
  AnimationEvent(int startFrame, int duration, std::function<void(double, double)> onTriggerCallback, std::function<void()> onEndCallback) :
    _frame(startFrame), _duration(duration), _onTrigger(onTriggerCallback), _onEnd(onEndCallback) {}
  void TriggerEvent(double x, double y) { _onTrigger(x, y); }
  void EndEvent() { _onEnd(); }
  int GetEndFrame() { return _frame + _duration; }

private:
  //Animation* _owner;
  //! Frame this event will be called on
  int _frame;
  int _duration;
  //!
  std::function<void(double, double)> _onTrigger;
  std::function<void()> _onEnd;
};

class Animation : public Image
{
public:
  Animation(const char* sheet, int rows, int columns, int startIndexOnSheet, int frames);

  void AddHitboxEvents(const char* hitboxesSheet, std::shared_ptr<Entity> entity);

  SDL_Rect GetFrameSrcRect(int frame);

  virtual void SetOp(const Transform& transform, SDL_Rect rectOnTex, Vector2<int> offset, bool flip, ResourceManager::BlitOperation* op) override;

  const int GetFrameCount() { return _frames; }

  Vector2<int> const GetFrameWH() { return _frameSize; }

  Vector2<int> const GetRefPxLocation() { return _referencePx; }

  void CheckEvents(int frame, double x, double y)
  {
    auto evtIter = _events.find(frame);
    if (evtIter != _events.end())
    {
      evtIter->second.TriggerEvent(x, y);
      _inProgressEvents.push_back(&evtIter->second);
    }

    for (int i = 0; i < _inProgressEvents.size(); i++)
    {
      AnimationEvent* evt = _inProgressEvents[i];
      if (frame == evt->GetEndFrame())
      {
        evt->EndEvent();
        _inProgressEvents.erase(_inProgressEvents.begin() + i);
        i--;
      }
    }
  }

protected:
  //! Gets first non-transparent pixel from the top left
  Vector2<int> FindReferencePixel(const char* sheet);
  //!
  int _rows, _columns, _frames, _startIdx;
  //!
  Vector2<int> _frameSize;
  //!
  Vector2<int> _referencePx;
  //!
  Vector2<int> _flipOffset;
  //! Map of frame starts for events to the event that should be triggered
  std::unordered_map<int, AnimationEvent> _events;
  std::vector<AnimationEvent*> _inProgressEvents;

};

class AnimationRenderer : public SpriteRenderer
{
public:
  AnimationRenderer(std::shared_ptr<Entity> owner);

  void RegisterAnimation(const std::string& name, const char* sheet, int rows, int columns, int startIndexOnSheet, int frames);

  virtual IDisplayable* GetDisplayable() override { return &_currentAnimation->second; }
  virtual SDL_Rect GetSourceRect() override
  {
    return _currentAnimation->second.GetFrameSrcRect(_frame);
  }
  virtual void Advance(float dt) override;
  virtual Vector2<int> GetDisplayOffset() const override {return _basisOffset;}

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
  bool IsPlaying() const { return _playing; }
  //!
  bool IsLooping() const { return _looping; }
  //!
  const std::string& GetAnimationName() const { return _currentAnimationName; }
  //!
  const std::function<int(int)>& GetNextFrame() const { return _nextFrameOp; }
  //!
  IAnimatorListener* GetListener() { return _listener; }

  //! MUTABLES
  //!
  int& GetCurrentFrame() { return _frame; }
  //!
  float& PlayTime() { return _accumulatedTime; }
  //!
  Animation& GetCurrentAnimation() { return _currentAnimation->second; }

  friend std::ostream& operator<<(std::ostream& os, const AnimationRenderer& animator);
  friend std::istream& operator>>(std::istream& is, AnimationRenderer& animator);

protected:
  IAnimatorListener* _listener;
  //!
  const float _secPerFrame = 1.0f / animation_fps;
  //!
  std::unordered_map<std::string, Animation> _animations;

  // STATE VARIABLES
  //!
  bool _playing;
  //!
  bool _looping;
  //!
  float _accumulatedTime;
  //!
  int _frame;
  //!
  std::string _currentAnimationName;
  //!
  std::unordered_map<std::string, Animation>::iterator _currentAnimation;
  
  //!
  Vector2<int> _basisOffset;
  //!
  Vector2<int> _basisRefPx;
  //!
  //Vector2<int> _basisRefSize;

  //!
  std::function<int(int)> _nextFrameOp;
  //
  std::function<int(int)> _loopAnimGetNextFrame = [this](int framesToAdv) { return (_frame + framesToAdv) % _currentAnimation->second.GetFrameCount(); };
  //
  std::function<int(int)> _onceAnimGetNextFrame = [this](int framesToAdv)
  {
    if ((_frame + framesToAdv) >= _currentAnimation->second.GetFrameCount())
    {
      for (auto& func : _onAnimCompleteCallbacks)
        func(&_currentAnimation->second);

      //stop playing and dont advance any frames
      _playing = false;
      return _currentAnimation->second.GetFrameCount() - 1;
    }
    return _frame + framesToAdv;
  };

  std::vector<std::function<void(Animation*)>> _onAnimCompleteCallbacks;

};

template <> struct ComponentTraits<AnimationRenderer>
{
  static const uint64_t GetSignature() { return 1 << 6;}
  //static const uint64_t GetSignature() { return 1 << 1;}
  //static const uint64_t GetSignature() { return ComponentTraits<Sprite>::GetSignature(); }
};