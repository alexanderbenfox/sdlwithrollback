#pragma once
#include "AssetManagement/Resource.h"
#include "IComponent.h"

const float animation_fps = 24.0f;

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
  Animation(const char* sheet, int rows, int columns, int frames) : _rows(rows), _columns(columns), _frames(frames), Image(sheet) {}

  SDL_Rect GetFrameSrcRect(int frame)
  {
    //if invalid frame, just return nothing
    if (frame >= _frames || frame < 0)
      return { 0, 0, 0, 0 };

    int x = frame % _columns;
    int y = frame / _columns;

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
  int _rows, _columns, _frames;

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
  Animator(std::shared_ptr<Entity> owner) : _playing(false), _accumulatedTime(0.0f), _frame(0), IComponent(owner) {}

  void Init(Animation* anim)
  {
    ResourceManager::Get().RegisterBlitOp();
    _display = std::unique_ptr<Animation>(anim);
  }

  virtual void OnFrameBegin() override
  {
    _op = ResourceManager::Get().GetAvailableOp();
  }

  virtual void Update(float dt) override
  {
    _accumulatedTime += dt;
    if (_accumulatedTime >= _secPerFrame)
    {
      int framesToAdv = (int)std::floorf(_accumulatedTime / _secPerFrame);
      
      // loop animation
      _frame = (_frame + framesToAdv) % _display->GetFrameCount();

      // 
      _accumulatedTime -= (framesToAdv * _secPerFrame);
    }
    _display->SetOp(_owner->transform, _display->GetFrameSrcRect(_frame), _op);
  }

protected:
  //!
  const float _secPerFrame = 1.0f / animation_fps;
  //!
  std::unique_ptr<Animation> _display;
  //! Blitter op used on this frame
  ResourceManager::BlitOperation* _op;

  //!
  bool _playing;
  //!
  float _accumulatedTime;
  //!
  int _frame;

};
