#pragma once
#include "AssetManagement/Resource.h"
#include "ResourceManager.h"
#include "IComponent.h"
#include "Transform.h"

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

class GraphicRenderer : public IComponent
{
public:


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
    _resource = std::unique_ptr<ResourceWrapper<SDL_Texture>>(new ResourceWrapper<SDL_Texture>(resource));
    sourceRect = { 0, 0, resource.GetInfo().mWidth, resource.GetInfo().mHeight };
  }

  void SetRenderResource(Resource<SDL_Texture>& resource)
  {
    _resource.reset();
    _resource = std::unique_ptr<ResourceWrapper<SDL_Texture>>(new ResourceWrapper<SDL_Texture>(resource));
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
  std::unique_ptr<ResourceWrapper<SDL_Texture>> _resource;

};

class GLGraphicRenderer : public IComponent
{
public:
  GLGraphicRenderer(std::shared_ptr<Entity> owner) : sourceRect{ 0, 0, 0, 0 }, IComponent(owner)
  {
    ResourceManager::Get().RegisterBlitOp();
  }

  ~GLGraphicRenderer()
  {
    ResourceManager::Get().DeregisterBlitOp();
  }

  //! Init with a resource
  void Init(Resource<GLTexture>& resource)
  {
    _resource = std::unique_ptr<ResourceWrapper<GLTexture>>(new ResourceWrapper<GLTexture>(resource));
    sourceRect = { 0, 0, resource.GetInfo().mWidth, resource.GetInfo().mHeight };
  }

  void SetRenderResource(Resource<GLTexture>& resource)
  {
    _resource.reset();
    _resource = std::unique_ptr<ResourceWrapper<GLTexture>>(new ResourceWrapper<GLTexture>(resource));
  }

  Resource<GLTexture>* GetRenderResource()
  {
    if (_resource)
      return &_resource->GetResource();
    return nullptr;
  }

  //! Source of display location on texture
  SDL_Rect sourceRect;

protected:
  //!
  std::unique_ptr<ResourceWrapper<GLTexture>> _resource;

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
    }
  }

  std::vector<GLDrawOperation> GetRenderOps()
  {
    return _string;
  }

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
