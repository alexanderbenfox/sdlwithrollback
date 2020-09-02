#pragma once
#include "Core/ECS/Entity.h"
#include "Core/ECS/IComponent.h"
#include "Core/Geometry2D/Rect.h"

//______________________________________________________________________________
//!
struct Transform : public IComponent, public ISerializable
{
  Transform();

  Vector2<float> position;
  Vector2<float> scale;
  Vector2<float> rotation;
  //! Rect used for anchor points and rendering images in the world
  Rect<float> rect;

  void SetWidthAndHeight(float width, float height);

  void Serialize(std::ostream& os) const override;
  void Deserialize(std::istream& is) override;
  std::string Log() override;

};

template <> struct ComponentInitParams<Transform>
{
  Vector2<float> position;
  Vector2<float> scale;
  Vector2<float> rotation;
  Vector2<float> size;
  static void Init(Transform& component, const ComponentInitParams<Transform>& params)
  {
    component.position = params.position;
    component.scale = params.scale;
    component.rotation = params.rotation;
    component.SetWidthAndHeight(params.size.x, params.size.y);
  }
};
