#pragma once
#include "AssetManagement/Resource.h"
#include "IComponent.h"

#include <functional>

class IDisplayable
{
public:
  virtual SDL_Rect GetRectOnSrcText() = 0;
  virtual void SetOp(const Transform& transform, SDL_Rect rectOnTex, ResourceManager::BlitOperation* op) = 0;
};

class Image : public IDisplayable
{
public:
  Image(const char* src) : _src(src)
  {
    auto size = ResourceManager::Get().GetTextureWidthAndHeight(src);
    _sourceRect = { 0, 0, size.x, size.y };
  }

  virtual SDL_Rect GetRectOnSrcText() override { return _sourceRect; }

  virtual void SetOp(const Transform& transform, SDL_Rect rectOnTex, ResourceManager::BlitOperation* op) override
  {
    op->_textureRect = rectOnTex;
    op->_textureResource = &ResourceManager::Get().GetTexture(_src);

    op->_displayRect = {
      static_cast<int>(std::floorf(transform.position.x)), static_cast<int>(std::floorf(transform.position.y)),
      (int)(static_cast<float>(_sourceRect.w) * transform.scale.x),
      (int)(static_cast<float>(_sourceRect.h) * transform.scale.y) };

    op->valid = true;
  }

protected:
  std::string _src;
  //! Source location on texture of sprite
  SDL_Rect _sourceRect;

};

class Animation : public Image
{
public:
  Animation(const char* sheet, int rows, int columns, int startIndexOnSheet, int frames) : _rows(rows), _columns(columns), _startIdx(startIndexOnSheet), _frames(frames), Image(sheet) {}

  SDL_Rect GetFrameSrcRect(int frame)
  {
    //if invalid frame, just return nothing
    if (frame >= _frames || frame < 0)
      return { 0, 0, 0, 0 };

    int x = (_startIdx + frame) % _columns;
    int y = (_startIdx + frame) / _columns;

    int fWidth = _sourceRect.w / _columns;
    int fHeight = _sourceRect.h / _rows;

    return { x * fWidth, y * fHeight, fWidth, fHeight };
  }

  virtual void SetOp(const Transform& transform, SDL_Rect rectOnTex, ResourceManager::BlitOperation* op) override
  {
    op->_textureRect = rectOnTex;
    op->_textureResource = &ResourceManager::Get().GetTexture(_src);

    int fWidth = _sourceRect.w / _columns;
    int fHeight = _sourceRect.h / _rows;

    op->_displayRect = {
      static_cast<int>(std::floorf(transform.position.x)), static_cast<int>(std::floorf(transform.position.y)),
      (int)(static_cast<float>(fWidth) * transform.scale.x),
      (int)(static_cast<float>(fHeight) * transform.scale.y) };

    op->valid = true;
  }

  const int GetFrameCount() { return _frames; }

protected:
  //!
  int _rows, _columns, _frames, _startIdx;

};

class Sprite : public IComponent
{
public:
  Sprite(std::shared_ptr<Entity> owner) : IComponent(owner) {}

  void Init(const char* sheet);

  virtual void OnFrameBegin() override;

  virtual void Update(float dt) override;

protected:
  //!
  std::unique_ptr<IDisplayable> _display;
  //! Blitter op used on this frame
  ResourceManager::BlitOperation* _op;

};


class Animator : public IComponent
{
public:
  Animator(std::shared_ptr<Entity> owner) : _playing(false), _accumulatedTime(0.0f), _frame(0), _nextFrameOp([](int) { return 0; }), IComponent(owner) {}

  void Init()
  {
    ResourceManager::Get().RegisterBlitOp();
  }

  void RegisterAnimation(const std::string& name, const char* sheet, int rows, int columns, int startIndexOnSheet, int frames)
  {
    if (_animations.find(name) == _animations.end())
    {
      _animations.insert(std::make_pair(name, Animation(sheet, rows, columns, startIndexOnSheet, frames)));
    }
  }

  virtual void OnFrameBegin() override
  {
    _op = ResourceManager::Get().GetAvailableOp();
  }

  virtual void Update(float dt) override
  {
    // if playing, do advance time and update frame
    if (_playing)
    {
      _accumulatedTime += dt;
      if (_accumulatedTime >= _secPerFrame)
      {
        int framesToAdv = (int)std::floorf(_accumulatedTime / _secPerFrame);

        // get next frame off of the type of anim it is
        _frame = _nextFrameOp(framesToAdv);

        // 
        _accumulatedTime -= (framesToAdv * _secPerFrame);
      }
    }

    _currentAnimation->second.SetOp(_owner->transform, _currentAnimation->second.GetFrameSrcRect(_frame), _op);
  }

  void Play(const std::string& name, bool isLooped)
  {
    // dont play again if we are already playing it
    if (_playing && name == _currentAnimationName) return;

    if (_animations.find(name) != _animations.end())
    {
      _currentAnimationName = name;
      _currentAnimation = _animations.find(name);
      _playing = true;

      // reset all parameters
      _accumulatedTime = 0;
      _frame = 0;

      if (isLooped)
        _nextFrameOp = [this](int i) { return _loopAnimGetNextFrame(i); };
      else
        _nextFrameOp = [this](int i) { return _onceAnimGetNextFrame(i); };
    }
  }

  Animation* GetAnimationByName(const std::string& name)
  {
    return &_animations.find(name)->second;
  }

  const Animation* GetCurrentlyPlaying()
  {
    return &_currentAnimation->second;
  }

  int GetShowingFrame() const { return _frame; }

protected:
  //!
  const float _secPerFrame = 1.0f / animation_fps;
  //!
  std::unordered_map<std::string, Animation> _animations;
  //! Blitter op used on this frame
  ResourceManager::BlitOperation* _op;

  //!
  bool _playing;


  //!
  float _accumulatedTime;
  //!
  int _frame;

  //!
  std::string _currentAnimationName;
  std::unordered_map<std::string, Animation>::iterator _currentAnimation;
  //
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
