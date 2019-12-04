#pragma once
#include "../include/AssetManagement/Resource.h"
#include "../include/GameManagement.h"
#include "../include/Entity.h"

template <typename T = IComponent>
class ComponentManager
{
public:
  static ComponentManager<T>& Get()
  {
    static ComponentManager<T> manager;
    return manager;
  }

  T* Create()
  {
    _components.push_back(T());
    return &_components.back();
  }

private:
  //
  std::vector<T> _components;
  //
  ComponentManager() {}
  ComponentManager(const ComponentManager&) = delete;
  ComponentManager<T> operator=(ComponentManager&) = delete;
};

class Sprite : public IComponent, IDrawable
{
public:
  Sprite() : IComponent() {}

  void Init(const char* sheet);

  virtual void Update(Transform& transform, float dt) override {}
  virtual void PushToRenderer(const Transform& transform) override;

protected:
  //! Source location on texture of sprite
  SDL_Rect _sourceRect;
  //!
  ResourceManager::BlitOperation* _blitter;

};

static bool SDLRectOverlap(const SDL_Rect& a, const SDL_Rect& b)
{
  return (b.x + b.w) >= a.x && b.x <= (a.x + a.w) &&
    (b.y + b.h) >= a.y && b.y <= (a.y + a.h);
}

class Camera : public IComponent
{
public:
  Camera() : IComponent() {}


  void Init(int w, int h);

  virtual void Update(Transform& transform, float dt) override;

  void ConvScreenSpace(ResourceManager::BlitOperation* entity);

  bool EntityInDisplay(const ResourceManager::BlitOperation* entity);

private:
  //!
  SDL_Rect _rect;
};
