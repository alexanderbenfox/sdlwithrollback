#pragma once
#include "Components/IComponent.h"
#include "Core/Geometry2D/Rect.h"
#include "Entity.h"

//______________________________________________________________________________
//!
struct Transform : public IComponent
{
  Transform();

  Vector2<float> position;
  Vector2<float> scale;
  Vector2<float> rotation;
  //! Rect used for anchor points and rendering images in the world
  Rect<float> rect;

  void SetWidthAndHeight(float width, float height);

  friend std::ostream& operator<<(std::ostream& os, const Transform& transform);
  friend std::istream& operator>>(std::istream& is, Transform& transform);

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
