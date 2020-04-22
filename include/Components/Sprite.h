#pragma once
#include "AssetManagement/Resource.h"
#include "IComponent.h"
#include "Transform.h"

#include <functional>
#include <cmath>

class GraphicRenderer : public IComponent
{
public:
  class TextureWrapper
  {
  public:
    TextureWrapper(Resource<SDL_Texture>& resource) : _resource(resource) {}
    Resource<SDL_Texture>& GetResource() { return _resource; }
  private:
    Resource<SDL_Texture>& _resource;
  };

  GraphicRenderer(std::shared_ptr<Entity> owner) : sourceRect{ 0, 0, 0, 0 }, IComponent(owner)
  {
    ResourceManager::Get().RegisterBlitOp();
  }

  ~GraphicRenderer()
  {
    ResourceManager::Get().DeregisterBlitOp();
  }

  //! Init with a resource
  void Init(Resource<SDL_Texture>& resource)
  {
    _resource = std::unique_ptr<TextureWrapper>(new TextureWrapper(resource));
    sourceRect = { 0, 0, resource.GetInfo().mWidth, resource.GetInfo().mHeight };
  }

  void SetRenderResource(Resource<SDL_Texture>& resource)
  {
    _resource.reset();
    _resource = std::unique_ptr<TextureWrapper>(new TextureWrapper(resource));
  }

  Resource<SDL_Texture>* GetRenderResource()
  {
    if(_resource)
      return &_resource->GetResource();
    return nullptr;
  }

  //! Source of display location on texture
  SDL_Rect sourceRect;
    
protected:
  //!
  std::unique_ptr<TextureWrapper> _resource;

};

class RenderProperties : public IComponent
{
public:
  RenderProperties(std::shared_ptr<Entity> owner) : offset(0, 0), horizontalFlip(false), _displayColor{ 255, 255, 255, SDL_ALPHA_OPAQUE }, IComponent(owner)
  {
  }

  virtual void SetDisplayColor(Uint8 r, Uint8 g, Uint8 b)
  {
    _displayColor = { r, g, b, SDL_ALPHA_OPAQUE };
  }
  virtual void SetDisplayColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
  {
    _displayColor = { r, g, b, a };
  }

  virtual SDL_Color GetDisplayColor() const
  {
    return _displayColor;
  }

  //! Display offset from top left of texture
  Vector2<int> offset;
  //!
  bool horizontalFlip;

protected:
  //!
  SDL_Color _displayColor;
  
};
