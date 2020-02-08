#pragma once
#include "Components/Sprite.h"
#include "ListenerInterfaces.h"

class Animation : public Image
{
public:
  Animation(const char* sheet, int rows, int columns, int startIndexOnSheet, int frames);

  SDL_Rect GetFrameSrcRect(int frame);

  virtual void SetOp(const Transform& transform, SDL_Rect rectOnTex, Vector2<int> offset, bool flip, ResourceManager::BlitOperation* op) override;

  const int GetFrameCount() { return _frames; }

  Vector2<int> const GetFrameWH() { return _frameSize; }

  Vector2<int> const GetRefPxLocation() { return _referencePx; }

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
  Vector2<int> _basisRefSize;

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
      return _frame;
    }
    return _frame + framesToAdv;
  };

  std::vector<std::function<void(Animation*)>> _onAnimCompleteCallbacks;

};

template <> struct ComponentTraits<AnimationRenderer>
{
  static const uint64_t GetSignature() { return 1 << 6;}
  //static const uint64_t GetSignature() { return ComponentTraits<Sprite>::GetSignature(); }
};