#pragma once
#include "Core/ECS/IComponent.h"

#include "AssetManagement/Resource.h"
#include "Rendering/RenderManager.h"

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
  RenderComponent() : sourceRect{ 0, 0, 0, 0 }, IComponent() {}

  void OnAdd(const EntityID& entity) override
  {
    RenderManager::Get().template RegisterDrawable<BlitOperation<TextureType>>(RenderLayer::World);
  }

  void OnRemove(const EntityID& entity) override
  {
    RenderManager::Get().template DeregisterDrawable<BlitOperation<TextureType>>(RenderLayer::World);
  }

  //! Init with a resource
  void Init(Resource<TextureType>& resource)
  {
    _resource = std::shared_ptr<ResourceWrapper<TextureType>>(new ResourceWrapper<TextureType>(resource));
    sourceRect = DrawRect<float>(0, 0, resource.GetInfo().mWidth, resource.GetInfo().mHeight);
  }

  void SetRenderResource(Resource<TextureType>& resource)
  {
    if (_resource == nullptr || &_resource->GetResource() != &resource)
    {
      _resource.reset();
      _resource = std::shared_ptr<ResourceWrapper<TextureType>>(new ResourceWrapper<TextureType>(resource));
    }
  }

  Resource<TextureType>* GetRenderResource()
  {
    if(_resource)
      return &_resource->GetResource();
    return nullptr;
  }

  //! Source of display location on texture
  DrawRect<float> sourceRect;
    
protected:
  //!
  std::shared_ptr<ResourceWrapper<TextureType>> _resource;

};

class TextRenderer : public IComponent
{
public:
  TextRenderer();
  void OnRemove(const EntityID& entity) override;

  void SetFont(LetterCase& resource);
  //! Sets up GL calls for text rendering and returns the size of the new on screen text field
  Vector2<float> SetText(const std::string& text, TextAlignment alignment, int fieldWidth = 600);

  std::vector<GLDrawOperation> GetRenderOps();

protected:
  //!
  LetterCase* _resource;
  //!
  std::vector<GLDrawOperation> _string;
  //!
  std::string _currentText;

};

class RenderProperties : public IComponent, ISerializable
{
public:
  RenderProperties();

  //! inherent offset added to any display from this entity
  Vector2<int> baseRenderOffset;
  //! Display offset from top left of texture to top left of transform
  Vector2<int> offset;
  //! Should the display be flipped horizontally
  bool horizontalFlip;
  //! Size of horizontal segment within an image defines the part that is to be displayed (used for flipping horizontally)
  float unscaledRenderWidth;

  virtual void SetDisplayColor(Uint8 r, Uint8 g, Uint8 b);
  virtual void SetDisplayColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
  virtual SDL_Color GetDisplayColor() const;
  Vector2<int> Offset() const;

  void Serialize(std::ostream& os) const override
  {
    os << baseRenderOffset;
    os << offset;
    Serializer<bool>::Serialize(os, horizontalFlip);
    Serializer<SDL_Color>::Serialize(os, _displayColor);
  }

  void Deserialize(std::istream& is) override
  {
    is >> baseRenderOffset;
    is >> offset;
    Serializer<bool>::Deserialize(is, horizontalFlip);
    Serializer<SDL_Color>::Deserialize(is, _displayColor);
  }

  std::string Log() override
  {
    std::stringstream ss;
    ss << "RenderProperties: \n";
    ss << "\tBase Render Offset: " << baseRenderOffset.x << " " << baseRenderOffset.y << "\n";
    ss << "\tOffset: " << offset.x << " " << offset.y << "\n";
    ss << "\tHorizontal flip: " << horizontalFlip << "\n";
    ss << "\tColor: " << _displayColor.r << " " << _displayColor.g << " " << _displayColor.b << " " << _displayColor.a << "\n";
    return ss.str();
  }

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
