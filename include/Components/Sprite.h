#pragma once
#include "AssetManagement/Resource.h"
#include "IComponent.h"

#include <functional>
#include <cmath>

class IDisplayable
{
public:
  virtual ~IDisplayable() {}
  virtual SDL_Rect GetRectOnSrcText() = 0;
  virtual void SetOp(const Transform& transform, SDL_Rect rectOnTex, Vector2<int> offset, ResourceManager::BlitOperation* op) = 0;
};

class Image : public IDisplayable
{
public:
  Image(const char* src) : _texture(ResourceManager::Get().GetTexture(src))
  {
    auto size = ResourceManager::Get().GetTextureWidthAndHeight(src);
    _sourceRect = { 0, 0, size.x, size.y };
  }

  virtual SDL_Rect GetRectOnSrcText() override { return _sourceRect; }

  virtual void SetOp(const Transform& transform, SDL_Rect rectOnTex, Vector2<int> offset, ResourceManager::BlitOperation* op) override
  {
    op->_textureRect = rectOnTex;
    op->_textureResource = &_texture;

    op->_displayRect = OpSysConv::CreateSDLRect(
      static_cast<int>(std::floor(transform.position.x - offset.x)),
      static_cast<int>(std::floor(transform.position.y - offset.y)),
      (int)(static_cast<float>(_sourceRect.w) * transform.scale.x),
      (int)(static_cast<float>(_sourceRect.h) * transform.scale.y));

    op->valid = true;
  }

protected:
  //std::string _src;
  Texture& _texture;
  //! Source location on texture of sprite
  SDL_Rect _sourceRect;

};

class Animation : public Image
{
public:
  Animation(const char* sheet, int rows, int columns, int startIndexOnSheet, int frames);

  SDL_Rect GetFrameSrcRect(int frame);

  virtual void SetOp(const Transform& transform, SDL_Rect rectOnTex, Vector2<int> offset, ResourceManager::BlitOperation* op) override;

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

};

class Sprite : public IComponent
{
public:
  Sprite(std::shared_ptr<Entity> owner) : _op(nullptr), IComponent(owner) {}

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
  Animator(std::shared_ptr<Entity> owner);

  void Init();

  void RegisterAnimation(const std::string& name, const char* sheet, int rows, int columns, int startIndexOnSheet, int frames);

  virtual void OnFrameBegin() override;

  virtual void Update(float dt) override;

  void Play(const std::string& name, bool isLooped);

  Animation* GetAnimationByName(const std::string& name);

  const Animation* GetCurrentlyPlaying();

  int GetShowingFrame() const { return _frame; }

  friend std::ostream& operator<<(std::ostream& os, const Animator& animator);
  friend std::istream& operator>>(std::istream& is, Animator& animator);

protected:
  //!
  const float _secPerFrame = 1.0f / animation_fps;
  //!
  std::unordered_map<std::string, Animation> _animations;
  //! Blitter op used on this frame
  ResourceManager::BlitOperation* _op;

  // STATE VARIABLES
  //!
  bool _playing;
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
