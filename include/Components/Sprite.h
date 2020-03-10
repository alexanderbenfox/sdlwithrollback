#pragma once
#include "AssetManagement/Resource.h"
#include "IComponent.h"
#include "Transform.h"

#include <functional>
#include <cmath>

class IDisplayable
{
public:
  virtual ~IDisplayable() {}
  virtual SDL_Rect GetRectOnSrcText() = 0;
  virtual void SetOp(const Transform& transform, SDL_Rect rectOnTex, Vector2<int> offset, bool flip, ResourceManager::BlitOperation* op) = 0;
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

  virtual void SetOp(const Transform& transform, SDL_Rect rectOnTex, Vector2<int> offset, bool flip, ResourceManager::BlitOperation* op) override
  {
    op->_textureRect = rectOnTex;
    op->_textureResource = &_texture;

    op->_displayRect = OpSysConv::CreateSDLRect(
      static_cast<int>(std::floor(transform.position.x - offset.x)),
      static_cast<int>(std::floor(transform.position.y - offset.y)),
      (int)(static_cast<float>(_sourceRect.w) * transform.scale.x),
      (int)(static_cast<float>(_sourceRect.h) * transform.scale.y));

    op->_flip = flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    op->valid = true;
  }

protected:
  //std::string _src;
  Texture& _texture;
  //! Source location on texture of sprite
  SDL_Rect _sourceRect;

};

class SpriteRenderer : public IComponent
{
public:
  SpriteRenderer(std::shared_ptr<Entity> owner) : _horizontalFlip(false), IComponent(owner)
  {
    ResourceManager::Get().RegisterBlitOp();
  }

  void Init(const char* sheet, bool horizontalFlip);

  virtual IDisplayable* GetDisplayable() {return _display.get();}
  virtual SDL_Rect GetSourceRect()
  {
    return GetDisplayable()->GetRectOnSrcText();
  }
  virtual void Advance(float dt) {}
  virtual bool const& GetFlip() const {return _horizontalFlip;}
  virtual Vector2<int> GetDisplayOffset() const { return Vector2<int>(); }

    
protected:
  //!
  std::unique_ptr<IDisplayable> _display;
  //! Blitter op used on this frame
  //ResourceManager::BlitOperation* _op;

  //!
  bool _horizontalFlip;

};

template <> struct ComponentTraits<SpriteRenderer>
{
  static const uint64_t GetSignature() { return 1 << 1;}
};
