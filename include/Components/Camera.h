#pragma once
#include "IComponent.h"
#include "Core/Math/Matrix4.h"
#include "AssetManagement/BlitOperation.h"

//!
static bool SDLRectOverlap(const SDL_Rect& a, const SDL_Rect& b)
{
  return (b.x + b.w) >= a.x && b.x <= (a.x + a.w) &&
    (b.y + b.h) >= a.y && b.y <= (a.y + a.h);
}

//!
class Camera : public IComponent, public DebugItem
{
public:
  //!
  Camera(std::shared_ptr<Entity> entity) : IComponent(entity), DebugItem("Camera") {}
  //!
  void Init(int w, int h);
  //!
  template <typename TextureType>
  void ConvScreenSpace(BlitOperation<TextureType>* entity);
  //!
  template <typename TextureType>
  bool EntityInDisplay(const BlitOperation<TextureType>* entity);
  //!
  virtual void OnDebug() override
  {
    ImGui::InputFloat("Zoom level", &zoom, 0.1f, 1.0f, 2);
  }
  //!
  SDL_Rect rect;
  //!
  Matrix4F matrix;
  //! takes care of the z value not in the transform component
  float zoom = 1.0f;

};

//______________________________________________________________________________
template <typename TextureType>
inline void Camera::ConvScreenSpace(BlitOperation<TextureType>* entity)
{
  entity->displayRect.x -= rect.x;
  entity->displayRect.y -= rect.y;
}

//______________________________________________________________________________
template <typename TextureType>
inline bool Camera::EntityInDisplay(const BlitOperation<TextureType>* entity)
{
  return SDLRectOverlap(rect, entity->displayRect);
}
