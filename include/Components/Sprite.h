#pragma once
#include "AssetManagement/Resource.h"
#include "IComponent.h"
#include "Transform.h"

#include <functional>
#include <cmath>

class GraphicRenderer : public IComponent
{
public:
  GraphicRenderer(std::shared_ptr<Entity> owner) : IComponent(owner)
  {
    ResourceManager::Get().RegisterBlitOp();
  }

  ~GraphicRenderer()
  {
    ResourceManager::Get().DeregisterBlitOp();
  }

  //! Init with a resource
  void SetRenderResource(Resource<SDL_Texture>& resource)
  {
    _resource = &resource;
    sourceRect = { 0, 0, _resource->GetInfo().mWidth, _resource->GetInfo().mHeight };
  }

  Resource<SDL_Texture>* GetRenderResource() { return _resource; }

  //! Source of display location on texture
  SDL_Rect sourceRect;
    
protected:
  //!
  Resource<SDL_Texture>* _resource;
  

};

class RenderProperties : public IComponent
{
public:
  RenderProperties(std::shared_ptr<Entity> owner) : _horizontalFlip(false), _displayColor{ 255, 255, 255, SDL_ALPHA_OPAQUE }, IComponent(owner)
  {
  }

  virtual bool const& GetFlip() const {return _horizontalFlip;}

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

protected:
  //!
  bool _horizontalFlip;
  //!
  SDL_Color _displayColor;
  

};

template <> struct ComponentTraits<GraphicRenderer>
{
  static const uint64_t GetSignature() { return 1 << 1;}
};
