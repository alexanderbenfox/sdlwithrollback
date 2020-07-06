#pragma once
#include "AssetManagement/Resource.h"
#include "Rendering/RenderManager.h"
#include "IComponent.h"
#include "Transform.h"
#include "AssetManagement/Text.h"

#include <functional>
#include <cmath>

template <typename T>
class ResourceWrapper
{
public:
  ResourceWrapper(Resource<T>& resource) : _resource(resource) {}
  Resource<T>& GetResource() { return _resource; }
private:
  Resource<T>& _resource;
};

template <typename TextureType>
class RenderComponent : public IComponent
{
public:
  RenderComponent(std::shared_ptr<Entity> owner) : sourceRect{ 0, 0, 0, 0 }, IComponent(owner)
  {
    RenderManager<TextureType>::Get().template RegisterDrawable<BlitOperation<TextureType>>();
  }

  ~RenderComponent()
  {
    RenderManager<TextureType>::Get().template DeregisterDrawable<BlitOperation<TextureType>>();
  }

  //! Init with a resource
  void Init(Resource<TextureType>& resource)
  {
    _resource = std::unique_ptr<ResourceWrapper<TextureType>>(new ResourceWrapper<TextureType>(resource));
    sourceRect = { 0, 0, resource.GetInfo().mWidth, resource.GetInfo().mHeight };
  }

  void SetRenderResource(Resource<TextureType>& resource)
  {
    if (_resource == nullptr || &_resource->GetResource() != &resource)
    {
      _resource.reset();
      _resource = std::unique_ptr<ResourceWrapper<TextureType>>(new ResourceWrapper<TextureType>(resource));
    }
  }

  Resource<TextureType>* GetRenderResource()
  {
    if(_resource)
      return &_resource->GetResource();
    return nullptr;
  }

  //! Source of display location on texture
  SDL_Rect sourceRect;
    
protected:
  //!
  std::unique_ptr<ResourceWrapper<TextureType>> _resource;

};

class TextRenderer : public IComponent
{
public:
  TextRenderer(std::shared_ptr<Entity> owner);
  ~TextRenderer();

  void SetFont(LetterCase& resource);

  void SetText(const std::string& text);

  std::vector<GLDrawOperation> GetRenderOps();

protected:
  //!
  LetterCase* _resource;
  //!
  std::vector<GLDrawOperation> _string;
  //!
  std::string _currentText;

};

class RenderProperties : public IComponent
{
public:
  RenderProperties(std::shared_ptr<Entity> owner);

  //! inherent offset added to any display from this entity
  Vector2<int> baseRenderOffset;
  //! Display offset from top left of texture to top left of transform
  Vector2<int> offset;
  //!
  bool horizontalFlip;

  virtual void SetDisplayColor(Uint8 r, Uint8 g, Uint8 b);
  virtual void SetDisplayColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
  virtual SDL_Color GetDisplayColor() const;
  Vector2<int> Offset() const;

protected:
  //!
  SDL_Color _displayColor;
  
};

template <> struct ComponentInitParams<RenderProperties>
{
  Vector2<int> offsetFromCenter;
  Uint8 r = 255;
  Uint8 g = 255;
  Uint8 b = 255;
  Uint8 a = 255;
  static void Init(RenderProperties& component, const ComponentInitParams<RenderProperties>& params)
  {
    component.baseRenderOffset = params.offsetFromCenter;
    component.SetDisplayColor(params.r, params.g, params.b, params.a);
  }
};
