#pragma once
#include "../include/AssetManagement/Resource.h"
#include "../include/GameManagement.h"
#include "../include/Entity.h"

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

static bool SDLRectOverlap(const SDL_Rect& a, const SDL_Rect& b)
{
  return (b.x + b.w) >= a.x && b.x <= (a.x + a.w) &&
    (b.y + b.h) >= a.y && b.y <= (a.y + a.h);
}

class Camera : public IComponent
{
public:
  Camera(std::shared_ptr<Entity> entity) : IComponent(entity) {}

  void Init(int w, int h);

  virtual void Update(float dt) override;

  void ConvScreenSpace(ResourceManager::BlitOperation* entity);

  bool EntityInDisplay(const ResourceManager::BlitOperation* entity);

private:
  //!
  SDL_Rect _rect;
};
