#pragma once
#include "IComponent.h"

//______________________________________________________________________________
//!
struct Transform : public IComponent
{
  Transform(std::shared_ptr<Entity> owner) :
    position(Vector2<float>(0.0f, 0.0f)),
    scale(Vector2<float>(1.0f, 1.0f)),
    rotation(Vector2<float>(0.0f, 0.0f)),
    IComponent(owner) {}
  Vector2<float> position;
  Vector2<float> scale;
  Vector2<float> rotation;

  friend std::ostream& operator<<(std::ostream& os, const Transform& transform);
  friend std::istream& operator>>(std::istream& is, Transform& transform);
};
