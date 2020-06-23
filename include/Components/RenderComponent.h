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
    RenderManager<TextureType>::Get().RegisterBlitOp();
  }

  ~RenderComponent()
  {
    RenderManager<TextureType>::Get().DeregisterBlitOp();
  }

  //! Init with a resource
  void Init(Resource<TextureType>& resource)
  {
    _resource = std::unique_ptr<ResourceWrapper<TextureType>>(new ResourceWrapper<TextureType>(resource));
    sourceRect = { 0, 0, resource.GetInfo().mWidth, resource.GetInfo().mHeight };
  }

  void SetRenderResource(Resource<TextureType>& resource)
  {
    _resource.reset();
    _resource = std::unique_ptr<ResourceWrapper<TextureType>>(new ResourceWrapper<TextureType>(resource));
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
  TextRenderer(std::shared_ptr<Entity> owner) : _currentText(""), IComponent(owner)
  {
  }

  ~TextRenderer()
  {
  }

  void SetFont(LetterCase& resource)
  {
    _resource = &resource;
  }

  void SetText(const std::string& text)
  {
    if (_resource && text != _currentText)
    {
      _currentText = text;
      _string = _resource->CreateStringField(_currentText.c_str(), 600);

      float width = 0;
      float height = 0;
      for(auto& letter : _string)
      {
        width = std::max(letter.x + letter.texture->GetInfo().mWidth, width);
        height = std::max(letter.y + letter.texture->GetInfo().mHeight, height);
      }
      _rect = Rect<float>(0, 0, width, height);
    }
  }

  std::vector<GLDrawOperation> GetRenderOps()
  {
    return _string;
  }

  Vector2<float> GetStringSize()
  {
    return Vector2<float>(_rect.Width(), _rect.Height());
  }

protected:
  //!
  LetterCase* _resource;
  //!
  std::vector<GLDrawOperation> _string;
  //!
  std::string _currentText;
  //!
  Rect<float> _rect;

};


class RenderProperties : public IComponent
{
public:
  RenderProperties(std::shared_ptr<Entity> owner) :
    baseRenderOffset(0, 0), 
    offset(0, 0), 
    horizontalFlip(false), 
    _displayColor{ 255, 255, 255, SDL_ALPHA_OPAQUE }, 
    IComponent(owner)
  {}

  //! inherent offset added to any display from this entity
  Vector2<int> baseRenderOffset;
  //! Display offset from top left of texture to top left of transform
  Vector2<int> offset;
  //!
  bool horizontalFlip;

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

  Vector2<int> Offset() const { return baseRenderOffset + offset; }

protected:
  //!
  SDL_Color _displayColor;
  
};
