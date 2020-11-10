#pragma once
#include "Core/ECS/IComponent.h"

#include "AssetManagement/Resource.h"
#include "Rendering/RenderManager.h"

#include "Transform.h"
#include "AssetManagement/Text.h"
#include "AssetManagement/EditableAssets/AnimationAsset.h"

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

  Vector2<float> renderScaling = Vector2<float>(1.0f, 1.0f);
  //! Defines the anchor points on the rendered image
  Vector2<float> rectTransform;
  //! Display offset from top left of texture to anchor point of the rect transform
  Vector2<float> offset;
  //! 
  AnchorPoint anchor = AnchorPoint::TL;
  //! Should the display be flipped horizontally
  bool horizontalFlip;

  virtual void SetDisplayColor(Uint8 r, Uint8 g, Uint8 b);
  virtual void SetDisplayColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
  virtual SDL_Color GetDisplayColor() const;

  void Serialize(std::ostream& os) const override
  {
    os << renderScaling;
    os << rectTransform;
    os << offset;
    Serializer<AnchorPoint>::Serialize(os, anchor);
    Serializer<bool>::Serialize(os, horizontalFlip);
    Serializer<SDL_Color>::Serialize(os, _displayColor);
  }

  void Deserialize(std::istream& is) override
  {
    is >> renderScaling;
    is >> rectTransform;
    is >> offset;
    Serializer<AnchorPoint>::Deserialize(is, anchor);
    Serializer<bool>::Deserialize(is, horizontalFlip);
    Serializer<SDL_Color>::Deserialize(is, _displayColor);
  }

  std::string Log() override
  {
    std::stringstream ss;
    ss << "RenderProperties: \n";
    ss << "\tRender Scaling: " << renderScaling.x << " " << renderScaling.y << "\n";
    ss << "\tRect Transform Bounds: " << rectTransform.x << " " << rectTransform.y << "\n";
    ss << "\tOffset: " << offset.x << " " << offset.y << "\n";
    ss << "\tAnchor: " << std::to_string(anchor) << "\n";
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
    component.offset = params.offsetFromCenter;
    component.SetDisplayColor(params.r, params.g, params.b, params.a);
  }
};
