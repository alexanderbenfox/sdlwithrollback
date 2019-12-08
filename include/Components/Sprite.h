#pragma once
#include "AssetManagement/Resource.h"
#include "IComponent.h"

class Sprite : public IComponent
{
public:
  Sprite(std::shared_ptr<Entity> owner) : IComponent(owner) {}

  void Init(const char* sheet);

  virtual void OnFrameBegin() override;

  virtual void Update(float dt) override;

protected:
  //! Source location on texture of sprite
  SDL_Rect _sourceRect;
  //!
  Texture* _textureResource;
  //! Blitter op used on this frame
  ResourceManager::BlitOperation* _op;

};
